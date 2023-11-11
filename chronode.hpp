#pragma once

// TODO: Make these optional (and a toggle for cmake).
#include "tabulate.hpp"

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
#define CHRONODE_MILLITIMER_INIT(cn, n) chronode::MilliTimer cn(n);
#define CHRONODE_START(cn, n) CHRONODE cn.start(n);
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

namespace util {
	auto& indent(std::ostream& os, size_t depth, const std::string& str="\t") {
		for(size_t i = 0; i < depth; i++) os << str;

		return os;
	}
}

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

/* template<typename Duration>
constexpr auto is_duration() {
	return std::is_same_v<
		Duration,
		std::chrono::duration<typename Duration::rep, typename Duration::period>
	>;
} */

template<typename Duration>
constexpr bool is_duration_v = std::is_same_v<
	Duration,
	std::chrono::duration<typename Duration::rep, typename Duration::period>
>;

// template<typename Duration, typename = std::enable_if_t<is_duration<Duration>()>>
template<typename Duration, std::enable_if_t<is_duration_v<Duration>, bool> = true>
class Node {
public:
	using duration_t = Duration;
	using node_t = Node<duration_t>;

	using Children = std::deque<node_t*>;
	// using Children = std::vector<node_t*>;
	// using Path = std::deque<const std::string&>;

	// https://en.cppreference.com/w/cpp/chrono/time_point
	struct time_point: public Clock::time_point {
		using Clock::time_point::time_point;
		using Clock::time_point::operator=;
		using Clock::time_point::time_since_epoch;

		constexpr time_point(const Clock::time_point& tp):
		Clock::time_point::time_point(tp) {
		}

		constexpr auto count() const {
			return std::chrono::time_point_cast<duration_t>(*this).time_since_epoch().count();
		}

		constexpr bool valid() const {
			return time_since_epoch() != Clock::duration::zero();
		}
	};

	constexpr Node(std::string_view name, const Node* parent=nullptr):
	_parent(parent),
	_c(0),
	_name(name) {
	}

	// TODO: I'd prefer making this private, and doing an explicity copy() method.
	constexpr Node(const Node& n):
	_parent(nullptr),
	_c(n._c),
	_start(n._start),
	_stop(n._stop),
	_name(n._name) {
		for(auto* c : n._children) {
			_children.emplace_back(new Node(*c));

			_children.back()->_parent = this;
		}
	}

	// TODO: This breaks C++14/17 if dtor is constexpr.
	// constexpr ~Node() {
	~Node() {
		for(auto* c : _children) delete c;
	}

	/* // TODO: See copy-constructor above.
	constexpr auto copy() const {
		if(_parent) throw exception("Cannot copy a parented Node.");

		return Node(*this);
	} */

	constexpr Node& reset() {
		_start = _stop = time_point{};
		_c = 0;

		for(auto* c : _children) c->reset();

		return *this;
	}

	constexpr auto& child(std::string_view name) {
		if(_c >= _children.size()) _children.push_back(new node_t(name, this));

		else _children[_c]->_name = name;

		_c++;

		return *_children[_c - 1];
	}

	constexpr auto& sleep(Clock::duration::rep r) {
		sleep_for(duration_t(r));

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
		auto d = std::chrono::duration_cast<duration_t>(_stop - _start).count();

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

	constexpr auto name() const {
		return _name;
	}

#if 0
	// Returns the full "path" from this Node to the topmost parent (the first Node it
	// finds with NO parent), in the correct order. It accepts a separator string, defaulting to the
	// traditional "/" character.
	constexpr auto path(std::string_view sep="/") const {
		Path ids_;

		const node_t* n = this;

		while(n) {
			ids_.push_front(&(n->_name));

			n = n->_parent;
		}

		// TODO: It SHOULD be safe to let the compiler do RVO here.
		// return std::move(ids_);
		return ids_;
	}
#endif

	constexpr auto start_point() const {
		return _start.count();
	}

	constexpr auto stop_point() const {
		return _stop.count();
	}

	friend std::ostream& operator<<(std::ostream& os, const node_t& n) {
		os << n._name << " " << n.duration() << duration_str<duration_t>();

		if(n._parent) os
			<< " ("
			<< std::setprecision(2)
			<< std::setiosflags(std::ios::fixed)
			<< (static_cast<double>(n.duration()) / static_cast<double>(n._parent->duration())) * 100.0
			<< "%)"
		;

		os
			<< " [@=" << &n
			<< ", parent=" << (n._parent ? n._parent->name() : "")
			<< "@" << n._parent
			<< ", children=" << n._children.size()
			<< "] {" << n._start.count() << " -> +"
			<< n._stop.count() - n._start.count() << "}"
		;

		return os;
	}

private:
	Node& operator=(const Node&) = delete;

	const Node* _parent;

	Children _children;
	typename Children::size_type _c;

	time_point _start;
	time_point _stop;

	std::string_view _name;
};

using NanoNode = Node<std::chrono::nanoseconds>;
using MicroNode = Node<std::chrono::microseconds>;
using MilliNode = Node<std::chrono::milliseconds>;

// template<typename Duration, typename = std::enable_if_t<is_duration<Duration>()>>
template<typename Duration, std::enable_if_t<is_duration_v<Duration>, bool> = true>
class Timer {
public:
	using node_t = Node<Duration>;

	Timer(std::string_view name):
	_node(name) {
	}

	void start(std::string_view name) {
		// If this is the FIRST start, go ahead and start our "toplevel" timer, which will hold this
		// first (requested) timer AND all subsequent children.
		//
		// TODO: The constructor currently sets this... but SHOULD it?
		if(!_n) _n = &_node.start();

		_n = &_n->child(name).start();
	}

	void stop() {
		// This is the "stop()" action that corresponds to the implicit "start."
		// TODO: This needs to be improved.
		if(!_n) _node.stop();

		else if(!_n->duration()) {
			_n = const_cast<node_t*>(_n->stop().parent());
		}
	}

	auto& node() {
		return _node;
	}

	const auto& node() const {
		return _node;
	}

private:
	node_t _node;
	node_t* _n = nullptr;
};

using NanoTimer = Timer<std::chrono::nanoseconds>;
using MicroTimer = Timer<std::chrono::microseconds>;
using MilliTimer = Timer<std::chrono::milliseconds>;

// TODO: Like Node and Timer, this should have its own is_node<>() check.
template<typename Node>
class Profile {
public:
	using node_t = Node;
	using data_t = std::deque<node_t>;

	Profile(size_t size):
	_size(size) {
	}

	constexpr void add(const node_t& node) {
		if(_data.size() >= _size) _data.pop_back();

		_data.push_front(node);
	}

	constexpr const auto& data() const {
		return _data;
	}

	constexpr size_t size() const {
		return _data.size();
	}

protected:
	size_t _size;

	data_t _data;
};

using NanoProfile = Profile<NanoNode>;
using MicroProfile = Profile<MicroNode>;
using MilliProfile = Profile<MilliNode>;

namespace report {
	namespace util {
		auto& indent(std::ostream& os, size_t depth, const std::string& str="\t") {
			for(size_t i = 0; i < depth; i++) os << str;

			return os;
		}
	}

	template<typename Duration>
	void ostream(const Node<Duration>& n, std::ostream& os, size_t depth=0) {
		for(size_t i = 0; i < depth; i++) os << "  ";

		if(depth) os << "\\_";

		os << n << std::endl;

		for(const auto& c : n.children()) ostream(*c, os, depth + 1);
	}

	template<typename Duration>
	void ostream_json(const Node<Duration>& n, std::ostream& os, size_t depth=0, bool comma=false) {
		auto ind = [&os, depth](size_t ex=0) -> auto& { return util::indent(os, depth + ex); };

		ind() << "{" << std::endl;
		ind(1) << "\"name\": \"" << n.name() << "\"," << std::endl;
		ind(1) << "\"unit\": \"" << duration_str<Duration>() << "\"," << std::endl;
		ind(1) << "\"start\": " << n.start_point() << "," << std::endl;
		ind(1) << "\"stop\": " << n.stop_point() << "," << std::endl;
		ind(1) << "\"children\": [" << std::endl;

		for(size_t i = 0; i < n.children().size(); i++) ostream_json(
			*n.children()[i],
			os,
			depth + 2,
			i < n.children().size() - 1
		);

		ind(1) << "]" << std::endl;
		ind() << "}" << (comma ? "," : "") << std::endl;
	}

	template<typename Node>
	void ostream_json(const Profile<Node>& p, std::ostream& os) {
		os << "[" << std::endl;

		size_t i = 0;

		for(const auto& n : p.data()) {
			ostream_json(n, os, 1, i >= p.size() - 1 ? false : true);

			i++;
		}

		os << "]" << std::endl;
	}

	/* template<typename Node>
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
	} */

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
#endif
}

}
