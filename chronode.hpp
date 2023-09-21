#pragma once

// TODO: Make this optional (and a toggle for cmake).
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
#define CHRONODE_START(cn) CHRONODE cn.start()
#define CHRONODE_STOP(cn) CHRONODE cn.stop()

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

// Ring Buffer: manages a static-sized array (of size N), permitting new values
// by always replacing the oldest.
template<typename T, size_t N>
class ring_buffer {
public:
	using buffer_t = std::array<T, N>;

	// constexpr ring_buffer(): _data{T()} { fmt::print("ctor0\n"); }
	// constexpr ring_buffer(const T& d): _data{} { fmt::print("ctor1\n"); }
	// constexpr ring_buffer(std::initializer_list<T> d): _data(d) { fmt::print("ctor1\n"); }
	// constexpr ring_buffer(const buffer_t& d): _data(d) { fmt::print("ctor2\n"); }

	constexpr void add(const T& t) {
		_data[_i % N] = t;

		_i++;

		// If we roll over, make sure on the second iteration we begin at a value that won't
		// make the buffer look "underfull."
		if(_i == std::numeric_limits<size_t>::max()) _i = N;
	}

	constexpr const auto& data() const {
		return _data;
	}

	constexpr size_t size() const {
		return _i >= N ? N : _i;
	}

protected:
	buffer_t _data;

	size_t _i = 0;
};

#define RING_BUFFER_T(_T, _N) \
	using ring_buffer_t = ring_buffer<_T, _N>; \
	using ring_buffer_t::_i; \
	using ring_buffer_t::_data; \
	using ring_buffer_t::size;

// Arithmatic Ring Buffer: manages a static-sized array (of size N), permitting new values
// by always replacing the oldest. Provides an "average" method for determining the mean
// of all values (and skips indices that are not yet populated, if applicable).
template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
class aring_buffer: public ring_buffer<T, N> {
public:
	RING_BUFFER_T(T, N)

	constexpr auto average() const {
		if(!_i) return T(0);

		// We use begin() + size() to make sure we only use valid, "add()'ed" values.
		// Otherwise, it might skew the average, factoring in a bunch of unwanted zeros.
		return std::accumulate(
			_data.begin(),
			_data.begin() + size(),
			T(0)
		) / static_cast<T>(size());
	}
};

using DEFAULT_ID = std::string;

constexpr size_t DEFAULT_SAMPLES = 5;

template<typename Unit, typename Id=DEFAULT_ID, size_t Samples=DEFAULT_SAMPLES>
class Node {
public:
	using unit_t = Unit;
	using id_t = Id;
	using node_t = Node<unit_t, id_t>;

	using Children = std::deque<node_t*>;
	// using Children = std::vector<node_t*>;
	using Ids = std::deque<const id_t*>;

	struct time_point: public Clock::time_point {
		using Clock::time_point::time_point;
		using Clock::time_point::operator=;
		using Clock::time_point::time_since_epoch;

		constexpr time_point(const Clock::time_point& tp): Clock::time_point::time_point(tp) {}

		constexpr auto count() const {
			return time_since_epoch().count();
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

	// constexpr ~Node() {
	~Node() {
		for(auto* c : _children) delete c;
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

	// TODO: This is dumb...
	template<typename T>
	constexpr auto& sleep(const T& t) {
		sleep_for(t);

		return *this;
	}

	// TODO: Check if started and throw exception!
	constexpr auto& start() {
		if(_start.valid()) throw exception("timer already started");

		/* _children.emplace_back(name, this);

		return _children.back(); */

		_start = tick();

		return *this;
	}

	// TODO: Check if stopped and throw exception!
	// TODO: Add a boolean check that stops this time AND all of its parents; this would
	// mean that the _parent member can no longer be 'const'.
	constexpr auto& stop() {
		_stop = tick();

		_average.add(duration());

		return *this;
	}

	// TODO: Do NOT return count(), but instead the duration object (since libfmt understands
	// this and can handle it better.
	constexpr auto duration(bool average=false) const {
		auto d = std::chrono::duration_cast<unit_t>(_stop - _start).count();

		if(average) d = _average.average();

		return d < 0 ? 0 : d;
	}

	// TODO: DELETE ME!
	constexpr void AVERAGES() const {
		for(decltype(_average.size()) i = 0; i < _average.size(); i++) {
			std::cout << _average.data()[i] << std::endl;
		}
	}

	/* // TODO: Check if started/stopped and throw exception!
	constexpr auto& call(const auto& func) {
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

	constexpr const auto& start_point() const {
		return _start;
	}

	constexpr const auto& stop_point() const {
		return _stop;
	}

	friend std::ostream& operator<<(std::ostream& os, const node_t& n) {
		os << n.id() << " " << n.duration();

		if(n.parent()) os
			<< " ("
			<< std::setprecision(2)
			<< std::setiosflags(std::ios::fixed)
			<< (static_cast<double>(n.duration()) / static_cast<double>(n.parent()->duration())) * 100.0
			<< "%)"
		;

		os
			<< " [id=" << n.id()
			<< ", parent=" << (n.parent() ? n.parent()->id() : "")
			<< ", children=" << n.children().size()
			<< "] {" << n._start.count() << " -> +"
			<< n._stop.count() - n._start.count() << "}"
		;

		return os;
	}

private:
	Node(const Node&) = delete;
	Node& operator=(const Node&) = delete;

	const Node* _parent;

	Children _children;
	typename Children::size_type _c;

	time_point _start;
	time_point _stop;

	aring_buffer<Clock::duration::rep, Samples> _average;

	id_t _id;
};

template<size_t Samples=DEFAULT_SAMPLES>
using NanoNode = Node<std::chrono::nanoseconds, DEFAULT_ID, Samples>;

template<size_t Samples=DEFAULT_SAMPLES>
using MicroNode = Node<std::chrono::microseconds, DEFAULT_ID, Samples>;

template<size_t Samples=DEFAULT_SAMPLES>
using MilliNode = Node<std::chrono::milliseconds, DEFAULT_ID, Samples>;

// TODO: Should I make this INHERIT from Node?
// TODO: Why am I able to use "Node" as a valid argument name?
template<typename Node>
class Timer {
public:
	Timer(const typename Node::id_t& id):
	_node(id) {
	}

	void start(const typename Node::id_t& id) {
		// If this is the FIRST start, go ahead and start our "toplevel" timer, which will hold this
		// first (requested) timer AND all subsequent children.
		// TODO: Pass ctor Id into THIS START ONLY!
		if(!_n) _n = &_node.start();

		_n = &_n->child(id).start();
	}

	void stop() {
		if(!_n) _node.stop();

		else if(!_n->duration()) {
			_n = const_cast<Node*>(_n->stop().parent());
		}
	}

	Node& node() {
		return _node;
	}

private:
	Node _node;
	Node* _n = nullptr;
};

template<size_t Samples=DEFAULT_SAMPLES>
using NanoTimer = Timer<NanoNode<Samples>>;

template<size_t Samples=DEFAULT_SAMPLES>
using MicroTimer = Timer<MicroNode<Samples>>;

template<size_t Samples=DEFAULT_SAMPLES>
using MilliTimer = Timer<MilliNode<Samples>>;

namespace report {
	template<typename Node>
	void ostream(const Node& n, std::ostream& os, size_t depth=0) {
		for(size_t i = 0; i < depth; i++) os << "  ";

		if(depth) os << "\\_";

		os << n << std::endl;

		for(const auto& c : n.children()) ostream(*c, os, depth + 1);
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
			std::to_string(n.start_point().count()),
			"+" + std::to_string(n.stop_point().count() - n.start_point().count())
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
}

}
