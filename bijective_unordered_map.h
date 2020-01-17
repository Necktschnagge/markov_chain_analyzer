#pragma once
#include <unordered_map>
#include <memory>
#include <stdexcept>

template <class _Kty,
	class _Ty,
	class _Hasher = std::hash<_Kty>,
	class _Keyeq = std::equal_to<_Kty>,
	class _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>
	class bijective_unordered_map
{
	/*
	template <class _Kty,
		class _Ty,
		class _Hasher = std::hash<_Kty>,
		class _Keyeq = std::equal_to<_Kty>,
		class _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>
	struct model {
		using forward_map = std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq, _Alloc>;
		using inverse_map = std::unordered_map< _Ty, _Kty, _Hasher, _Keyeq, _Alloc>;
		forward_map forward;
		inverse_map inverse;
	}
	*/
	using forward_map = std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq, _Alloc>;
	using inverse_map = std::unordered_map< _Ty, _Kty, _Hasher, _Keyeq, _Alloc>;
	std::shared_ptr<forward_map> forward;
	std::shared_ptr<inverse_map> inverse;

	bijective_unordered_map(const std::shared_ptr<forward_map>& forward, std::shared_ptr<inverse_map> inverse) :
		forward(forward), inverse(inverse) {}

public:

	bijective_unordered_map() {}
	bijective_unordered_map<_Kty, _Ty, _Hasher, _Keyeq, _Alloc> getInverse() {
		return bijective_unordered_map<_Kty, _Ty, _Hasher, _Keyeq, _Alloc>::bijective_unordered_map(inverse, forward);
	}

	const _Ty& get(const _Kty& key) {
		return (*forward)[key];
	}

	void set(const _Kty& key, const _Ty value) {
		if (forward->find(key) != forward->end()) { // key has already entry in map
			_Ty old_value = (*forward)[key];
			if (value == old_value) return;
			if (inverse->find(value) != inverse->end()) throw std::invalid_argument("You cannot map a key to some value such that there exists another key mapped to the same value.");
			(*forward)[key] = value;
			inverse->erase(old_value);
			(*inverse)[value] = key;
		}
		else { // new key is added
			if (inverse->find(value) != inverse->end()) throw std::invalid_argument("You cannot map a key to some value such that there exists another key mapped to the same value.");
			(*forward)[key] = value;
			inverse[value] = key;
		}
	}

	void erase(const _Kty& key) {
		if (forward->find(key) == forward->end()) return;
		const _Ty& value{ (*forward)[key] };
		inverse->erase(value);
		forward->erase(key);
	}
};
