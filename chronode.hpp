#pragma once

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

	class JSONStream {
	public:
		// WHOA! Why the "comma" argument, you ask? Well, this is useful when iterating over a
		// group of json()'able things, where you often WANT a comma (except for the LAST element).
		// This could be done other ways, of course... but using it as a parameter simplifies the
		// caller's code (to a slight degree).
		virtual std::ostream& json(std::ostream& os, size_t depth=0, bool comma=false) const = 0;

		std::string json(size_t depth=0, bool comma=false) const {
			std::stringstream ss;

			json(ss, depth, comma);

			return ss.str();
		}

		friend std::ostream& operator<<(std::ostream& os, const JSONStream& js) {
			return js.json(os);
		}
	};

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
class Node: public util::JSONStream {
public:
	using duration_t = Duration;
	using node_t = Node<duration_t>;

	using Children = std::deque<node_t>;
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

	// TODO: It'll be necessary to have a default ctor if I want to use preallocation in Profile.
	/* constexpr Node():
	_parent(nullptr),
	_c(0) {
	} */

	constexpr Node(std::string_view name, const node_t* parent=nullptr):
	_parent(parent),
	_c(0),
	_name(name) {
	}

	// TODO: I'd prefer making this private, and doing an explicity copy() method.
	constexpr Node(const node_t& n):
	_parent(nullptr),
	_c(n._c),
	_start(n._start),
	_stop(n._stop),
	_name(n._name) {
		for(const auto& c : n._children) {
			_children.emplace_back(c);

			_children.back()._parent = this;
		}
	}

	/* // TODO: See copy-constructor above.
	constexpr auto copy() const {
		if(_parent) throw exception("Cannot copy a parented Node.");

		return Node(*this);
	} */

	// This IMMEDIATELY resets all the internal Node data; it's worth mentioning, because the
	// Timer's version of reset() does NOT.
	constexpr Node& reset() {
		_start = _stop = time_point{};
		_c = 0;

		for(auto& c : _children) c.reset();

		return *this;
	}

	constexpr auto& child(std::string_view name) {
		if(_c >= _children.size()) _children.emplace_back(name, this);

		else _children[_c]._name = name;

		_c++;

		return _children[_c - 1];
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

	// TODO: Do NOT return count(), but instead the duration object.
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

	virtual std::ostream& json(std::ostream& os, size_t depth=0, bool comma=false) const {
		auto ind = [&os, depth](size_t ex=0) -> auto& { return util::indent(os, depth + ex); };

		ind() << "{" << std::endl;
		ind(1) << "\"name\": \"" << _name << "\"," << std::endl;
		// ind(1) << "\"name\": \"" << _name << " (" << this << ")\"," << std::endl;
		ind(1) << "\"start\": " << _start.count() << "," << std::endl;
		ind(1) << "\"stop\": " << _stop.count() << "," << std::endl;
		// ind(1) << "\"parent\": \"" << (_parent ? _parent->name() : "null") << " (" << _parent << ")\"," << std::endl;
		ind(1) << "\"children\": [" << std::endl;

		for(size_t i = 0; i < _children.size(); i++) _children[i].json(
			os,
			depth + 2,
			i < _children.size() - 1
		);

		ind(1) << "]" << std::endl;
		ind() << "}" << (comma ? "," : "") << std::endl;

		return os;
	}

private:
	Node() = delete;
	Node& operator=(const Node&) = delete;

	const node_t* _parent;

	Children _children;
	typename Children::size_type _c;

	time_point _start;
	time_point _stop;

	std::string_view _name;
};

using NanoNode = Node<std::chrono::nanoseconds>;
using MicroNode = Node<std::chrono::microseconds>;
using MilliNode = Node<std::chrono::milliseconds>;

template<typename Node>
class Profile;

// template<typename Duration, typename = std::enable_if_t<is_duration<Duration>()>>
template<typename Duration, std::enable_if_t<is_duration_v<Duration>, bool> = true>
class Timer: public util::JSONStream {
public:
	using node_t = Node<Duration>;

	constexpr Timer(std::string_view name):
	_node(name) {
	}

	constexpr void start(std::string_view name) {
		// The ACTUAL CALL to _node.reset() is deferred until this point, so that after calling
		// the Timer's reset(), its "state" remains (and be queried or copied into a Profile).
		if(_reset) {
			_node.reset();

			_reset = false;
		}

		// If this is the FIRST start, go ahead and start our "toplevel" timer, which will hold this
		// first (requested) timer AND all subsequent children.
		//
		// TODO: The constructor currently sets this... but SHOULD it?
		if(!_n) _n = &_node.start();

		_n = &_n->child(name).start();
	}

	constexpr void stop() {
		// This is the "stop()" action that corresponds to the implicit "start."
		// TODO: Should this be done inside reset()?
		if(!_n) _node.stop();

		else if(!_n->duration()) {
			_n = const_cast<node_t*>(_n->stop().parent());
		}
	}

	// This calls the necessary final "stop" (which we need beceause of the implicit "start" before
	// creating the first child), and sets a boolean flag indicating that the next time this
	// object's start() method is called all of the internal Node data should be reset. This is
	// necessary so that AFTER calling reset() on this Timer, the caller can continue to safely
	// query the Node graph until they "start" again.
	//
	// Furthermore, we return a const reference to the internal node_t in its "final" state so that
	// external code can query any relevant information (or, in the case of Profile, make a copy for
	// analysis later).
	constexpr const auto& reset() {
		stop();

		_reset = true;

		return _node;
	}

	// TODO: This isn't enough; at a minimum, it needs to include the same extra fields that Profile
	// does (unit and duration).
	virtual std::ostream& json(std::ostream& os, size_t depth=0, bool comma=false) const {
		_node.json(os, depth, comma);

		return os;
	}

private:
	friend class Profile<node_t>;

	node_t _node;
	node_t* _n = nullptr;
	bool _reset = false;
};

using NanoTimer = Timer<std::chrono::nanoseconds>;
using MicroTimer = Timer<std::chrono::microseconds>;
using MilliTimer = Timer<std::chrono::milliseconds>;

// TODO: Like Node and Timer, this should have its own is_node<>() check.
template<typename Node>
class Profile: public util::JSONStream {
public:
	using node_t = Node;
	using duration_t = typename node_t::duration_t;
	using data_t = std::deque<node_t>;

	// TODO: It will likely be more performant to use the preallocation constructor for deque.
	Profile(size_t size):
	_size(size) {
	}

	// Copies an existing Node object into the data history.
	constexpr void add(const node_t& node) {
		if(_data.size() >= _size) _data.pop_back();

		_data.push_front(node);
	}

	// Copies the "managed" Node held inside a Timer object into the data history.
	//
	// TODO: Should this implictly call reset() on the Timer instance?
	constexpr void add(const Timer<duration_t>& timer) {
		add(timer._node);
	}

	virtual std::ostream& json(std::ostream& os, size_t depth=0, bool comma=false) const {
		auto ind = [&os, depth](size_t ex=0) -> auto& { return util::indent(os, depth + ex); };

		ind() << "{" << std::endl;
		ind(1) << "\"unit\": \"" << duration_str<duration_t>() << "\"," << std::endl;

		auto average = std::accumulate(
			_data.begin(),
			_data.end(),
			typename duration_t::rep(0),
			// [](typename duration_t::rep l, const node_t& r) {
			// TODO: Both Clang 12 and GCC 10 let me get away with using "auto" here in C++17 mode;
			// I'm not sure if that's a bug or a bonus. :)
			[](auto l, const auto& r) { return l + r.duration(); }
		) / static_cast<typename duration_t::rep>(_data.size());

		ind(1) << "\"duration\": " << average << "," << std::endl;
		ind(1) << "\"data\": [" << std::endl;

		for(size_t i = 0; i < _data.size(); i++) _data[i].json(
			os,
			2,
			i >= _data.size() - 1 ? false : true
		);

		ind(1) << "]" << std::endl;
		ind() << "}" << std::endl;

		return os;
	}

protected:
	size_t _size;

	data_t _data;
};

using NanoProfile = Profile<NanoNode>;
using MicroProfile = Profile<MicroNode>;
using MilliProfile = Profile<MilliNode>;

}
