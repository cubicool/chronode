#pragma once

// TODO: Make these optional (and a toggle for cmake).
#include "tabulate.hpp"
// #include "json.hpp"

#include <chrono>
#include <thread>
#include <utility>
// #include <vector>
#include <deque>
#include <sstream>
#include <numeric>
#include <iomanip>

#include <iostream>

#define CHRONODE if constexpr(chronode::ENABLED)
#define CHRONODE_MILLITIMER(cn) extern chronode::MilliTimer cn;
#define CHRONODE_MILLITIMER_INIT(cn, id) chronode::MilliTimer cn(id);
#define CHRONODE_START(cn, id) CHRONODE cn.start(id);
#define CHRONODE_STOP(cn) CHRONODE cn.stop();
#define CHRONODE_DATA(cn, dd) CHRONODE cn.data(d);

using namespace std::chrono_literals;

namespace chronode {

#ifdef CHRONODE_ENABLED
	constexpr bool ENABLED = true;

	// #pragma message "Chronode enabled!"

#else
	constexpr bool ENABLED = false;

	// #pragma message "Chronode disabled..."
#endif

using std::this_thread::sleep_for;
using Clock = std::chrono::steady_clock;

static constexpr auto tick = Clock::now;

class exception: public std::exception {
public:
	exception() = delete;

	/* template<typename... Args>
	exception(std::string_view s, Args&&... args) {
		// _err = fmt::format(s, std::forward<Args>(args)...);
	} */

	exception(const std::string& err):
	_err(err) {
	}

	virtual const char* what() const noexcept {
		return _err.c_str();
	}

	friend std::ostream& operator<<(std::ostream& os, const exception& e) {
		/* try {
			os << e.what();
		}

		// format_error, system_error, and runtime_error can come out of the fmt library.
		catch(const fmt::format_error&) {
			os << "fmt/formatting error";
		} */

		return os;
	}

private:
	std::string _err;
};

template<typename Duration>
constexpr std::string_view duration_str() {
// std::string duration_str() {
	if constexpr(std::is_same_v<Duration, std::chrono::nanoseconds>) return "ns";
	else if constexpr(std::is_same_v<Duration, std::chrono::microseconds>) return "us";
	else if constexpr(std::is_same_v<Duration, std::chrono::milliseconds>) return "ms";
	else if constexpr(std::is_same_v<Duration, std::chrono::seconds>) return "s";
	else if constexpr(std::is_same_v<Duration, std::chrono::minutes>) return "min";
	else if constexpr(std::is_same_v<Duration, std::chrono::hours>) return "hr";
	else return "unknown";
}

template<typename Unit, typename Id=std::string>
class Node {
public:
	using unit_t = Unit;
	using id_t = Id;
	using node_t = Node<unit_t, id_t>;

	using Children = std::deque<node_t*>;
	// using Children = std::vector<node_t*>;
	using Ids = std::deque<const id_t*>;

	// https://en.cppreference.com/w/cpp/chrono/time_point
	struct time_point: public Clock::time_point {
		using Clock::time_point::time_point;
		using Clock::time_point::operator=;
		using Clock::time_point::time_since_epoch;

		constexpr time_point(const Clock::time_point& tp):
		Clock::time_point::time_point(tp) {
		}

		constexpr auto count() const {
			return std::chrono::time_point_cast<unit_t>(*this).time_since_epoch().count();
		}

		constexpr bool valid() const {
			return time_since_epoch() != Clock::duration::zero();
		}
	};

	constexpr Node(const id_t& id_, const Node* parent=nullptr):
	_parent(parent),
	_c(0),
	_id(id_) {
	}

	// TODO: This breaks C++14/17 if dtor is constexpr.
	// constexpr ~Node() {
	~Node() {
		for(auto* c : _children) delete c;
	}

	constexpr auto copy() const {
		if(_parent) throw exception("Cannot copy a parented Node.");

		return Node(*this);
	}

	constexpr Node& reset() {
		_start = _stop = time_point{};
		_c = 0;

		for(auto* c : _children) c->reset();

		return *this;
	}

	constexpr auto& child(const id_t& id) {
		if(_c >= _children.size()) _children.push_back(new node_t(id, this));

		else _children[_c]->_id = id;

		_c++;

		return *_children[_c - 1];
	}

	constexpr auto& sleep(Clock::duration::rep r) {
		sleep_for(unit_t(r));

		return *this;
	}

	constexpr auto& sleep(Clock::duration d) {
		sleep_for(d);

		return *this;
	}

	// TODO: Check if started and throw exception!
	constexpr auto& start() {
		if(_start.valid()) throw exception("Timer already started.");

		_start = tick();

		return *this;
	}

	// TODO: Check if stopped and throw exception!
	// TODO: Add a boolean check that stops this time AND all of its parents; this would
	// mean that the _parent member can no longer be 'const'.
	constexpr auto& stop() {
		_stop = tick();

		return *this;
	}

	// TODO: Do NOT return count(), but instead the duration object (since libfmt understands
	// this and can handle it better.
	constexpr auto duration() const {
		auto d = std::chrono::duration_cast<unit_t>(_stop - _start).count();

		return d < 0 ? 0 : d;
	}

	// TODO: Offer a method that will return he "relative" duration; maybe this should be a
	// boolean parameter to duration() above?
	// (static_cast<double>(duration()) / static_cast<double>(_parent->duration())) * 100.0

	/* constexpr auto& call(const auto& func) {
		start();
		func();
		stop();

		return *this;
	} */

	// constexpr auto& parent() const {
	constexpr const auto* parent() const {
		return _parent;
	}

	constexpr auto& children() const {
		return _children;
	}

	constexpr const auto& id() const {
		return _id;
	}

	// Returns all Id values from this Node to the topmost parent (the first Node it
	// finds with NO parent), in the correct order.
	constexpr auto ids() const {
		Ids ids_;

		const node_t* n = this;

		while(n) {
			ids_.push_front(&(n->_id));

			n = n->_parent;
		}

		// TODO: It SHOULD be safe to let the compiler do RVO here.
		// return std::move(ids_);
		return ids_;
	}

	constexpr auto start_point() const {
		return _start.count();
	}

	constexpr auto stop_point() const {
		return _stop.count();
	}

	friend std::ostream& operator<<(std::ostream& os, const node_t& n) {
		os << n._id << " " << n.duration() << duration_str<unit_t>();

		if(n._parent) os
			<< " ("
			<< std::setprecision(2)
			<< std::setiosflags(std::ios::fixed)
			<< (static_cast<double>(n.duration()) / static_cast<double>(n._parent->duration())) * 100.0
			<< "%)"
		;

		os
			// << " [id=" << n._id
			<< " [@=" << &n
			<< ", parent=" << (n._parent ? n._parent->id() : "")
			<< "@" << n._parent
			<< ", children=" << n._children.size()
			<< "] {" << n._start.count() << " -> +"
			<< n._stop.count() - n._start.count() << "}"
		;

		return os;
	}

private:
	constexpr Node(const Node& n):
	_parent(nullptr),
	_c(n._c),
	_start(n._start),
	_stop(n._stop),
	_id(n._id) {
		for(auto* c : n._children) {
			_children.emplace_back(new Node(*c));

			_children.back()->_parent = this;
		}
	}

	Node& operator=(const Node&) = delete;

	const Node* _parent;

	Children _children;
	typename Children::size_type _c;

	time_point _start;
	time_point _stop;

	id_t _id;
};

using NanoNode = Node<std::chrono::nanoseconds>;
using MicroNode = Node<std::chrono::microseconds>;
using MilliNode = Node<std::chrono::milliseconds>;

// TODO: Should I make this INHERIT from Node?
// TODO: Why am I able to use "Node" as a valid argument name?
template<typename Node>
class Timer {
public:
	using node_t = Node;

	Timer(const typename node_t::id_t& id):
	_node(id) {
	}

	void start(const typename node_t::id_t& id) {
		// If this is the FIRST start, go ahead and start our "toplevel" timer, which will hold this
		// first (requested) timer AND all subsequent children.
		// TODO: Pass ctor Id into THIS START ONLY!
		if(!_n) _n = &_node.start();

		_n = &_n->child(id).start();
	}

	void stop() {
		// This is the "stop()" action that corresponds to the implicit "start."
		// TODO: This needs to be improved.
		if(!_n) _node.stop();

		else if(!_n->duration()) {
			_n = const_cast<Node*>(_n->stop().parent());
		}
	}

	auto& node() {
		return _node;
	}

	const auto& node() const {
		return _node;
	}

private:
	Node _node;
	Node* _n = nullptr;
};

using NanoTimer = Timer<NanoNode>;
using MicroTimer = Timer<MicroNode>;
using MilliTimer = Timer<MilliNode>;

namespace report {
	template<typename Node>
	void ostream(const Node& n, std::ostream& os, size_t depth=0) {
		for(size_t i = 0; i < depth; i++) os << "  ";

		if(depth) os << "\\_";

		os << n << std::endl;

		for(const auto& c : n.children()) ostream(*c, os, depth + 1);
	}

	template<typename Node>
	void ostream_json(const Node& n, std::ostream& os, size_t depth=0, bool comma=false) {
		auto indent = [&os, depth](size_t extra=0) -> std::ostream& {
			for(size_t i = 0; i < depth + extra; i++) os << "\t";

			return os;
		};

		indent() << "{" << std::endl;
		indent(1) << "\"id\": \"" << n.id() << "\"," << std::endl;
		indent(1) << "\"unit\": \"" << duration_str<typename Node::unit_t>() << "\"," << std::endl;
		indent(1) << "\"start\": " << n.start_point() << "," << std::endl;
		indent(1) << "\"stop\": " << n.stop_point() << "," << std::endl;
		indent(1) << "\"children\": [" << std::endl;

		for(size_t i = 0; i < n.children().size(); i++) ostream_json(
			*n.children()[i],
			os,
			depth + 2,
			i < n.children().size() - 1
		);

		indent(1) << "]" << std::endl;
		indent() << "}" << (comma ? "," : "") << std::endl;
	}

	template<typename Node>
	void tabulate(const Node& n, tabulate::Table& table, size_t depth=0) {
		std::ostringstream os;

		for(const auto* i : n.ids()) os << "/" << *i;

		table.add_row({
			n.id(),
			n.parent() ? n.parent()->id() : "",
			os.str(),
			std::to_string(n.children().size()),
			std::to_string(n.duration()),
			std::to_string(n.start_point()),
			"+" + std::to_string(n.stop_point() - n.start_point())
		});

		for(const auto* c : n.children()) tabulate(*c, table, depth + 1);
	}

	/* template<typename N>
	void format(...) {
		// TODO: This is the code for using libfmt.
		if(_parent) d = fmt::format("{}ms ({:.2f}%)",
			duration(),
			(static_cast<double>(duration()) / static_cast<double>(_parent->duration())) * 100.0
		);

		fmt::print("{}{} [name={}, parent={}, children={}] {{{} -> {}}}\n",
			indent,
			d,
			_name,
			_parent ? _parent->_name : "",
			_children.size(),
			_start.count(),
			_stop.count()
		);
	} */

#if 0
	template<typename Node>
	const nlohmann::json& json(const Node& n, nlohmann::json* j=nullptr, size_t depth=0) {
		thread_local static nlohmann::json _json{};

		if(!j) {
			_json = nlohmann::json{};

			j = &_json;
		}

		(*j)["id"] = n.id();
		(*j)["start"] = n.start_point().count();
		(*j)["stop"] = n.stop_point().count();
		(*j)["children"] = nlohmann::json::array();

		// for(const auto* c : n.children()) rows_spans(*c, dr, depth + 1);

		return _json;
	}
#endif

	using Span = std::pair<Clock::duration::rep, Clock::duration::rep>;
	using Spans = std::vector<Span>;
	using RowsSpans = std::vector<Spans>;

	// TODO: Should this be a method on Node instead?
	template<typename Node>
	// const auto& rows_spans(const Node& n, RowsSpans* dr=nullptr, size_t depth=0) {
	const RowsSpans& rows_spans(const Node& n, RowsSpans* dr=nullptr, size_t depth=0) {
		thread_local static RowsSpans _dr{};

		// The reason we pass a pointer to something that is statically available is because
		// it's ALSO USED as an indicator the RowsSpans object needs to be reset/cleared.
		if(!dr) {
			_dr.clear();

			dr = &_dr;
		}

		// if(dr->size() <= depth) dr->emplace_back(Spans{});
		// XXX: This is possible (instead of the above) because emplace_back will forward the
		// parameters to the underlying/templated type for us! Same below...
		if(dr->size() <= depth) dr->emplace_back();

		// (*dr)[depth].emplace_back(Span{n.start_point().count(), n.stop_point().count()});
		(*dr)[depth].emplace_back(n.start_point(), n.stop_point());

		std::cout
			<< "depth=" << depth
			<< " adding " << n.start_point()
			<< ", " << n.stop_point() << std::endl
		;

		for(const auto* c : n.children()) rows_spans(*c, dr, depth + 1);

		return _dr;
	}
}

}
