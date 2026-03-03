#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu
{
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector
{
private:
	static const int initial_size=4;
	static const int scaleup_rate=2;
	static T* temp;

	T* data;
	int siz,real_siz;

	void scaleUp()
	{
		temp=data;
		data=new T[real_siz*=scaleup_rate]();
		for(int i=0;i<siz;i++) data[i]=temp[i];
		delete[] temp;
	}
	void expendOne()
	{
		siz++;
		if(siz>real_siz) scaleUp();
	}

public:
	class const_iterator;
	class iterator
	{
	// The following code is written for the C++ type_traits library.
	// Type traits is a C++ feature for describing certain properties of a type.
	// For instance, for an iterator, iterator::value_type is the type that the
	// iterator points to.
	// STL algorithms and containers may use these type_traits (e.g. the following
	// typedef) to work properly. In particular, without the following code,
	// @code{std::sort(iter, iter1);} would not compile.
	// See these websites for more information:
	// https://en.cppreference.com/w/cpp/header/type_traits
	// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
	// About iterator_category: https://en.cppreference.com/w/cpp/iterator
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;

	private:
		vector<T> *bel;
		pointer pos;

	public:
		iterator(vector<T> *_bel,pointer _pos) noexcept:bel(_bel),pos(_pos){}
		iterator operator+(const int &n) const {return iterator(bel,pos+n);}
		iterator operator-(const int &n) const {return iterator(bel,pos-n);}
		int operator-(const iterator &rhs) const
		{
			if(bel!=rhs.bel) throw invalid_iterator();
			return pos-rhs.pos;
		}
		iterator& operator+=(const int &n){pos+=n;return *this;}
		iterator& operator-=(const int &n){pos-=n;return *this;}
		iterator  operator++(int) {pos++;return iterator(bel,pos-1);}
		iterator& operator++()    {pos++;return *this;}
		iterator  operator--(int) {pos--;return iterator(bel,pos+1);}
		iterator& operator--()    {pos--;return *this;}
		T& operator*() const{return *pos;}

		bool operator==(const       iterator &rhs) const {return pos==rhs.pos;}
		bool operator==(const const_iterator &rhs) const {return pos==rhs.pos;}
		bool operator!=(const       iterator &rhs) const {return !(this->operator==(rhs));}
		bool operator!=(const const_iterator &rhs) const {return !(this->operator==(rhs));}
	};
	class const_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;
		
	private:
		const vector<T> *bel;
		const pointer pos;

	public:
		const_iterator(vector<T> *_bel,pointer _pos) noexcept:bel(_bel),pos(_pos){}
		const_iterator operator+(const int &n) const {return const_iterator(bel,pos+n);}
		const_iterator operator-(const int &n) const {return const_iterator(bel,pos-n);}
		int operator-(const const_iterator &rhs) const
		{
			if(bel!=rhs.bel) throw invalid_iterator();
			return pos-rhs.pos;
		}
		const_iterator& operator+=(const int &n){pos+=n;return *this;}
		const_iterator& operator-=(const int &n){pos-=n;return *this;}
		const_iterator  operator++(int) {pos++;return const_iterator(bel,pos-1);}
		const_iterator& operator++()    {pos++;return *this;}
		const_iterator  operator--(int) {pos--;return const_iterator(bel,pos+1);}
		const_iterator& operator--()    {pos--;return *this;}
		const T& operator*() const{return *pos;}

		bool operator==(const       iterator &rhs) const {return pos==rhs.pos;}
		bool operator==(const const_iterator &rhs) const {return pos==rhs.pos;}
		bool operator!=(const       iterator &rhs) const {return !(this->operator==(rhs));}
		bool operator!=(const const_iterator &rhs) const {return !(this->operator==(rhs));}

	};
	/**
	 * TODO Constructs
	 * At least two: default constructor, copy constructor
	 */
	vector()
	{
		data=new T[real_siz=initial_size]();
		siz=0;
	}
	vector(const vector &other)
	{
		siz=other.size();
		data=new T[real_siz=max(initial_size,siz)]();
		for(int i=0;i<other.size();i++) data[i]=other[i];
	}
	~vector() {delete[] data;}
	vector &operator=(const vector &other)
	{
		if(other.begin()==begin())
		{
			siz=other.size(),real_siz=other.real_siz;
			return;
		}

		delete[] data;
		siz=other.size();
		data=new T[real_siz=max(initial_size,siz)]();
		for(int i=0;i<other.size();i++) data[i]=other[i];
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 */
	      T & at(const size_t &pos)       {if(pos<0||pos>=siz) throw index_out_of_bound();return data[pos];}
	const T & at(const size_t &pos) const {if(pos<0||pos>=siz) throw index_out_of_bound();return data[pos];}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 * !!! Pay attentions
	 *   In STL this operator does not check the boundary but I want you to do.
	 */
	      T & operator[](const size_t &pos)       {if(pos<0||pos>=siz) throw index_out_of_bound();return data[pos];}
	const T & operator[](const size_t &pos) const {if(pos<0||pos>=siz) throw index_out_of_bound();return data[pos];}

	const T & front() const {if(siz==0) throw container_is_empty();return data[0    ]};
	const T & back () const {if(siz==0) throw container_is_empty();return data[siz-1]};
	/**
	 * returns an iterator to the beginning.
	 */
	iterator        begin()       {return       iterator(this,data);}
	const_iterator  begin() const {return const_iterator(this,data);}
	const_iterator cbegin() const {return const_iterator(this,data);}
	/**
	 * returns an iterator to the end.
	 */
	iterator        end()       {return       iterator(this,data+siz);}
	const_iterator  end() const {return const_iterator(this,data+siz);}
	const_iterator cend() const {return const_iterator(this,data+siz);}
	/**
	 * checks whether the container is empty
	 */
	bool empty() const {return siz==0;}
	/**
	 * returns the number of elements
	 */
	size_t size() const {return siz;}
	/**
	 * clears the contents
	 */
	void clear() {siz=0;}
	/**
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value.
	 */
	iterator insert(iterator pos, const T &value)
	{
		int ind=pos-begin();
		return insert(ind,value);
	}
	/**
	 * inserts value at index ind.
	 * after inserting, this->at(ind) == value
	 * returns an iterator pointing to the inserted value.
	 * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
	 */
	iterator insert(const size_t &ind, const T &value)
	{
		if(ind>siz) throw index_out_of_bound();
		expendOne();
		for(iterator i=siz-1;i>=ind;i--) data[i+1]=data[i];
		data[ind]=value;
		return iterator(this,ind);
	}
	/**
	 * removes the element at pos.
	 * return an iterator pointing to the following element.
	 * If the iterator pos refers the last element, the end() iterator is returned.
	 */
	iterator erase(iterator pos)
	{
		int ind=pos-begin();
		return erase(ind);
	}
	/**
	 * removes the element with index ind.
	 * return an iterator pointing to the following element.
	 * throw index_out_of_bound if ind >= size
	 */
	iterator erase(const size_t &ind)
	{
		if(ind>=siz) throw index_out_of_bound();
		for(int i=ind;i<siz;i++) data[i]=data[i+1];
		siz--;
		return iterator(this,ind);
	}
	/**
	 * adds an element to the end.
	 */
	void push_back(const T &value)
	{
		insert(end(),value);
	}
	/**
	 * remove the last element from the end.
	 * throw container_is_empty if size() == 0
	 */
	void pop_back()
	{
		if(siz==0) throw container_is_empty();
		erase(end()-1);
	}
};


}

#endif
