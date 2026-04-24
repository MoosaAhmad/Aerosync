#pragma once
#include<iostream>
using namespace std;

template<typename T>
class Vector {
	T* ptr;
	int cap;
	int sz;

	void grow() {
		if (cap == 0) {
			ptr = new T[1];
			cap = 1;
		}
		else {
			T* tmp = new T[2 * cap];
			for (int i = 0;i < sz;++i) tmp[i] = ptr[i];
			delete[]ptr;
			ptr = tmp;
			cap *= 2;
		}
	}
public:

	//   Ctors & Dtor

	Vector() :ptr(nullptr), cap(0), sz(0) {};
	Vector(int sz, T val) :ptr(new T[sz]), cap(sz), sz(sz) {
		for (int i = 0;i < sz;++i) ptr[i] = val;
	}
	Vector(const Vector& other) : sz(other.sz), cap(other.cap), ptr(new T[cap]) {
		for (int i = 0; i < sz; ++i) ptr[i] = other.ptr[i];
	}
	Vector(Vector&& other) noexcept
		: ptr(other.ptr), sz(other.sz), cap(other.cap) {
		other.ptr = nullptr;
		other.sz = 0;
		other.cap = 0;
	}
	~Vector() { delete[]ptr; }

	//   Assignment Operators

	Vector& operator=(const Vector& other) {
		if (this != &other) {
			delete[] ptr;

			sz = other.sz;
			cap = other.cap;
			ptr = new T[cap];

			for (int i = 0; i < sz; ++i) ptr[i] = other.ptr[i];
		}
		return *this;
	}
	Vector& operator=(Vector&& other) noexcept {
		if (this != &other) {
			delete[] ptr;

			ptr = other.ptr;
			sz = other.sz;
			cap = other.cap;

			other.ptr = nullptr;
			other.sz = 0;
			other.cap = 0;
		}
		return *this;
	}


	//insert a value at the end
	void push_back(T val) {
		if (sz == cap) grow();
		ptr[sz++] = val;
	}
	// remove last value
	void pop_back() {
		if (sz <= 0) throw runtime_error("vector is already empty");
		--sz;
	}


	// to expand the container to a suitable bigger size to avoid repeatd growing
	void reserve(int _sz) {
		if (_sz < 0) throw invalid_argument("invalid parameter:_sz must be >0");
		if (_sz <= cap) return;

		T* tmp = new T[_sz];
		for (int i = 0;i < sz;++i) tmp[i] = ptr[i];
		delete[]ptr;
		ptr = tmp;
		cap = _sz;
	}




	// same as array[ ]
	T& operator [] (int idx) {
		if (idx < 0)throw runtime_error("invalid index access");
		if (idx >= sz) throw runtime_error("vector subscript out of range");
		return ptr[idx];
	}
	const T& operator[](int idx) const {
		if (idx < 0 || idx >= sz)
			throw runtime_error("vector subscript out of range");
		return ptr[idx];
	}


	// first element->if vector is not empty
	T& front() {
		if (sz == 0) throw runtime_error("empty vector!!");
		return ptr[0];
	}
	const T& front() const {
		if (sz == 0) throw runtime_error("empty Vector");
		return ptr[0];
	}
	// last element ->if !vector.empty
	T& back() {
		if (sz == 0) throw runtime_error("empty vector!");
		return ptr[sz - 1];
	}
	const T& back() const {
		if (sz == 0) throw runtime_error("empty vector");
		return ptr[sz - 1];
	}

	// the current holding capacity
	int capacity() const { return cap; }
	// number of items present
	int size() const { return sz; }

	void shrink_to_fit() {
		if (cap > sz) {
			if (sz == 0) {
				delete[]ptr;ptr = nullptr;cap = 0;return;
			}
			T* tmp = new T[sz];
			for (int i = 0;i < sz;++i) tmp[i] = ptr[i];
			delete[]ptr;
			ptr = tmp;
			cap = sz;
		}
	}
	bool empty() const { return sz == 0; }
	// to reset the vector
	void clear() { sz = 0; }
	// erase value from a particular idx
	void erase(int idx) {
		if (idx < 0 || idx >= sz)throw invalid_argument("vector subscript out of range");
		for (int i = idx;i < sz - 1;++i)ptr[i] = ptr[i + 1];
		--sz;
	}
	// insert some value at a particular index.
	void insert(int idx, T val) {
		if (idx < 0 || idx > sz)
			throw invalid_argument("invalid index");

		if (sz == cap) grow();

		for (int i = sz; i > idx; --i)
			ptr[i] = ptr[i - 1];

		ptr[idx] = val;
		++sz;
	}
};