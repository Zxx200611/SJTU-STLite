#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#pragma GCC optimize(2)

#include <cmath>       // in case you need it
#include <cstddef>     // for size_t
#include <functional>  // for std::less

#include<cassert>
// #include<iostream>
// #include<vector>

#include "exceptions.hpp"

namespace sjtu {

template<typename T> class vector;

/**
 * @brief A container automatically sorting its contents, similar to
 * std::priority_queue but with extra functionalities.
 *
 * The extra functionalities are:
 * - Merge two priority queues into one (with good time complexity).
 * - Clear all elements in the queue.
 * - Limited exception safety for some operations (e.g. push, pop, top, merge)
 * when the comparator throws exceptions from `Compare` only.
 *
 * This @priority_queue does not support passing an underlying container as a template parameter.
 * Also, it does not support passing a comparator object as a constructor argument.
 *
 */
template <class T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node
    {
        T val;
        Node* ch[24];
        int lvl;

        Node():val(),lvl(0)
		{
			for(int i=0;i<24;i++) ch[i]=nullptr;
		}
        Node(const Node &v):val(v.val),lvl(v.lvl)
		{
			for(int i=0;i<24;i++) ch[i]=v.ch[i];
		}
		Node(const T& _val):val(_val),lvl(0)
		{
			for(int i=0;i<24;i++) ch[i]=nullptr;
		}
    };
	struct ChainNode
	{
		Node *u;
		ChainNode *nxt;

		ChainNode():u(nullptr),nxt(nullptr){}
		ChainNode(Node *_u,ChainNode *_nxt):u(_u),nxt(_nxt){}
	};
	ChainNode *rts,*mnv_c,*mnv_pc;
    Node *mnv_p;
    int siz;
    Compare comp;

    void copyTree(Node *&u,Node *v)
    {
        if(v==nullptr)
        {
            u=nullptr;
            return;
        }

        u=new Node(*v);
        for(int i=0;i<v->lvl;i++) copyTree(u->ch[i],v->ch[i]);
    }
    void deleteTree(Node *u)
    {
        if(u==nullptr) return;

        for(int i=0;i<u->lvl;i++) deleteTree(u->ch[i]);
        delete u;
    }
	Node* mergeNode(Node *u,Node *v,vector<Node *> &oped)
	{
		if(v==nullptr) return u;
		if(u==nullptr) return v;

		assert(u->lvl==v->lvl);
		if(comp(u->val,v->val)) std::swap(u,v);
		u->ch[u->lvl++]=v,oped.push_back(u);
		return u;
	}

public:
    priority_queue():comp(),rts(nullptr),mnv_p(nullptr),mnv_c(nullptr),mnv_pc(nullptr),siz(0){};
    priority_queue(const priority_queue& v):priority_queue()
    {
        for(ChainNode *i=v.rts,*j=nullptr;i!=nullptr;i=i->nxt)
        {
            Node *u=nullptr;
            copyTree(u,i->u);
			ChainNode *p=new ChainNode(u,nullptr);

            if(v.mnv_p==i->u) mnv_p=u,mnv_c=p,mnv_pc=j;
			if(rts==nullptr) j=rts=p;
			else j->nxt=p,j=j->nxt;
        }
        siz=v.siz;
    }
    ~priority_queue()
	{
		for(ChainNode *i=rts,*j=nullptr;i!=nullptr;i=j)
		{
			deleteTree(i->u),j=i->nxt;
			delete i;
		}
		rts=nullptr;
	}

    priority_queue& operator=(const priority_queue& v)
	{
		if(this==&v) return *this;

		clear();

        for(ChainNode *i=v.rts,*j=nullptr;i!=nullptr;i=i->nxt)
        {
            Node *u=nullptr;
            copyTree(u,i->u);
			ChainNode *p=new ChainNode(u,nullptr);

            if(v.mnv_p==i->u) mnv_p=u,mnv_c=p,mnv_pc=j;
			if(rts==nullptr) j=rts=p;
			else j->nxt=p,j=j->nxt;
        }
        siz=v.siz;
		return *this;
	}

    /** Adds one element to the queue. */
    void push(const T& val)
	{
		priority_queue tmp;
		Node *v=new Node(val);
		tmp.rts=new ChainNode(v,nullptr);
		tmp.mnv_p=v,tmp.siz=1,tmp.mnv_c=tmp.rts,tmp.mnv_pc=nullptr;

		// std::cout<<"Going to merge"<<std::endl;
		merge(tmp);
		// std::cout<<"Done"<<std::endl;
	}

    /**
     * Returns a read-only reference of the first element in the queue.
     *
     * @throws container_is_empty when the first element does not exist.
     */
    const T& top() const
	{
		if(size()==0) throw container_is_empty();
		return mnv_p->val; 
	}

    /**
     * Removes the first element in the queue.
     *
     * @throws container_is_empty when the first element does not exist.
     */
    void pop()
	{
		if(size()==0) throw container_is_empty();
		
		ChainNode *it=mnv_c,*pre=mnv_pc;
		// for(pre=nullptr,it=rts;it!=nullptr;pre=it,it=it->nxt) if(it->u==mnv_p) break;
		assert(it!=nullptr);
		if(pre!=nullptr) pre->nxt=it->nxt;
		else rts=it->nxt;
		siz-=(1<<(mnv_p->lvl));

		priority_queue tmp;
		ChainNode *tmpp=nullptr;
		for(int i=0;i<mnv_p->lvl;i++)
		{
			Node *v=mnv_p->ch[i];
			if(tmp.rts==nullptr) tmpp=tmp.rts=new ChainNode(v,nullptr);
			else tmpp->nxt=new ChainNode(v,nullptr),tmpp=tmpp->nxt;
			tmp.siz+=(1<<(v->lvl));
		}
		
		try
		{
			Node *tmpmnvp=mnv_p;
			merge(tmp);
			delete tmpmnvp;
			delete it;
		}
		catch(...)
		{
			siz+=(1<<(mnv_p->lvl));
			if(pre!=nullptr) pre->nxt=it;
			else rts=it;

			for(ChainNode *i=tmp.rts,*j=nullptr;i!=nullptr;i=j)
			{
				j=i->nxt;
				delete i;
			}
			tmp.rts=nullptr;
			throw;
		}
	}

    /** Returns the number of elements in the queue. */
    size_t size() const {return siz;}

    /** Returns whether there is any element in the queue. */
    bool empty() const {return siz==0;}

    /** Clears all elements in the queue. */
    void clear()
	{
		for(ChainNode *i=rts,*j=nullptr;i!=nullptr;i=j)
		{
			deleteTree(i->u),j=i->nxt;
			delete i;
		}
		rts=nullptr;
		mnv_p=nullptr,siz=0,mnv_c=nullptr,mnv_pc=nullptr;
	}

    /**
     * @brief Merges two priority queues into one.
     *
     * The merged data shall be stored in the current priority queue and the
     * other priority queue shall be cleared after merging.
     *
     * The time complexity shall be O(log n) or better.
     */
    void merge(priority_queue& B)
	{
		if(this==&B) return;

		Node *up=nullptr,*new_mnv_p=nullptr;
		vector<Node *> oped;
		vector<ChainNode *> del,alloc_ed;
		ChainNode *new_rts=nullptr,*new_ed=nullptr,*new_mnv_c=nullptr,*new_mnv_pc=nullptr;

		// std::cout<<"Merge A { "<<std::endl;
		// debugPrint();
		// std::cout<<"}"<<std::endl;
		// std::cout<<"Merge B { "<<std::endl;
		// B.debugPrint();
		// std::cout<<"}"<<std::endl;

		try
		{
			ChainNode *i=rts,*j=B.rts;
			for(int l=0;i!=nullptr||j!=nullptr||up!=nullptr;l++)
			{
				// std::cout<<"Lvl = "<<l<<" : "<<std::endl;
				while(i!=nullptr&&i->u->lvl<l) i=i->nxt;
				while(j!=nullptr&&j->u->lvl<l) j=j->nxt;

				if(j==nullptr&&up==nullptr)
				{
					// std::cout<<"Break"<<std::endl;
					if(new_ed!=nullptr) new_ed->nxt=i;
					else new_rts=i;
					break;
				}

				if(up!=nullptr&&(i!=nullptr&&i->u->lvl==l)&&(j!=nullptr&&j->u->lvl==l))
				{
					// std::cout<<"Full add"<<std::endl;
					if(new_ed==nullptr) alloc_ed.push_back(new_ed=new_rts=new ChainNode(up,nullptr));
					else alloc_ed.push_back(new_ed->nxt=new ChainNode(up,nullptr)),new_ed=new_ed->nxt;
					up=mergeNode(i->u,j->u,oped);
					del.push_back(i),del.push_back(j);
					i=i->nxt,j=j->nxt;
				}
				else
				{
					// std::cout<<"Not Full add"<<std::endl;
					if(i!=nullptr&&i->u->lvl==l) up=mergeNode(up,i->u,oped),del.push_back(i),i=i->nxt;
					if(j!=nullptr&&j->u->lvl==l) up=mergeNode(up,j->u,oped),del.push_back(j),j=j->nxt;
					if(up!=nullptr&&up->lvl==l)
					{
						if(new_ed==nullptr) alloc_ed.push_back(new_ed=new_rts=new ChainNode(up,nullptr));
						else alloc_ed.push_back(new_ed->nxt=new ChainNode(up,nullptr)),new_ed=new_ed->nxt;
						// std::cout<<"Add a chainnode which root val = "<<up->val<<std::endl;
						up=nullptr;
					}
				}
			}

			for(ChainNode *i=new_rts,*j=nullptr;i!=nullptr;j=i,i=i->nxt)
			{
				if(new_mnv_p==nullptr||comp(new_mnv_p->val,i->u->val))
				{
					new_mnv_p=i->u;
					new_mnv_c=i;
					new_mnv_pc=j;
				}
			}
		}
		catch(...)
		{
			// std::cout<<"Error detected"<<std::endl;
			for(int i=(int)oped.size()-1;i>=0;i--)
			{
				oped[i]->lvl--;
				oped[i]->ch[oped[i]->lvl]=nullptr;
			}
			for(ChainNode *i:alloc_ed) delete i;
			throw;
		}

		rts=new_rts,siz+=B.siz,mnv_p=new_mnv_p,mnv_c=new_mnv_c,mnv_pc=new_mnv_pc;
		// debugPrint();
		for(ChainNode *i:del) delete i;
		B.rts=nullptr,B.siz=0,B.mnv_p=nullptr,B.mnv_c=nullptr,B.mnv_pc=nullptr;
	}
	// void debugPrint()
	// {
	// 	std::cout<<"Heap roots : *****************"<<std::endl;
	// 	for(ChainNode *i=rts;i!=nullptr;i=i->nxt)
	// 	{
	// 		std::cout<<"Node #"<<(unsigned long long)i<<" : ";
	// 		std::cout<<"lvl = "<<i->u->lvl<<" , ";
	// 		// std::cout<<"rt val = "<<i->u->val;
	// 		std::cout<<std::endl;
	// 	}
	// 	std::cout<<"******************************"<<std::endl;
	// }
};

#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector
{
private:
	static const int initial_size;
	static const int scaleup_rate;

	T* data;
	int siz,real_siz;

	void scaleUp()
	{
		T* temp=data;
		real_siz*=scaleup_rate;
		data=static_cast<T*>(::operator new[](real_siz*sizeof(T)));
		for(int i=0;i<siz;i++) new (data+i) T(temp[i]);
		for(int i=0;i<siz;i++) temp[i].~T();
		::operator delete[] (temp);
	}
	void expendOne()
	{
		if(siz+1>real_siz) scaleUp();
		siz++;
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
		vector<T>* belong() const {return bel;}
		pointer position() const {return pos;}
		iterator(vector<T> *_bel,pointer _pos) noexcept:bel(_bel),pos(_pos){}
		iterator operator+(const int &n) const {return iterator(bel,pos+n);}
		iterator operator-(const int &n) const {return iterator(bel,pos-n);}
		int operator-(const iterator &rhs) const
		{
			// if(bel!=rhs.belong()) throw invalid_iterator();
			assert(bel==rhs.belong());
			return pos-rhs.position();
		}
		iterator& operator+=(const int &n){pos+=n;return *this;}
		iterator& operator-=(const int &n){pos-=n;return *this;}
		iterator  operator++(int) {pos++;return iterator(bel,pos-1);}
		iterator& operator++()    {pos++;return *this;}
		iterator  operator--(int) {pos--;return iterator(bel,pos+1);}
		iterator& operator--()    {pos--;return *this;}
		T& operator*() const{return *pos;}

		bool operator==(const       iterator &rhs) const {return pos==rhs.position();}
		bool operator==(const const_iterator &rhs) const {return pos==rhs.position();}
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
		pointer pos;

	public:
		const vector<T>* belong() const {return bel;}
		pointer position() const {return pos;}
		const_iterator(const vector<T> *_bel,pointer _pos) noexcept:bel(_bel),pos(_pos){}
		const_iterator operator+(const int &n) const {return const_iterator(bel,pos+n);}
		const_iterator operator-(const int &n) const {return const_iterator(bel,pos-n);}
		int operator-(const const_iterator &rhs) const
		{
			// if(bel!=rhs.belong()) throw invalid_iterator();
			assert(bel==rhs.belong());
			return pos-rhs.position();
		}
		const_iterator& operator+=(const int &n){pos+=n;return *this;}
		const_iterator& operator-=(const int &n){pos-=n;return *this;}
		const_iterator  operator++(int) {pos++;return const_iterator(bel,pos-1);}
		const_iterator& operator++()    {pos++;return *this;}
		const_iterator  operator--(int) {pos--;return const_iterator(bel,pos+1);}
		const_iterator& operator--()    {pos--;return *this;}
		const T& operator*() const{return *pos;}

		bool operator==(const       iterator &rhs) const {return pos==rhs.position();}
		bool operator==(const const_iterator &rhs) const {return pos==rhs.position();}
		bool operator!=(const       iterator &rhs) const {return !(this->operator==(rhs));}
		bool operator!=(const const_iterator &rhs) const {return !(this->operator==(rhs));}

	};
	/**
	 * TODO Constructs
	 * At least two: default constructor, copy constructor
	 */
	vector()
	{
		// std::cout<<"Constructing"<<std::endl;
		real_siz=initial_size;
		data=static_cast<T*>(::operator new[](real_siz*sizeof(T)));
		siz=0;
		// std::cout<<"Done"<<std::endl;
	}
	vector(const vector &other)
	{
		siz=other.size();
		real_siz=std::max<int>(initial_size,siz);
		data=static_cast<T*>(::operator new[](real_siz*sizeof(T)));
		for(int i=0;i<other.size();i++) new (data+i) T(other[i]);
	}
	~vector()
	{
		for(int i=0;i<siz;i++) data[i].~T();
		::operator delete[] (data);
	}
	vector &operator=(const vector &other)
	{
		if(this==&other) return *this;

		for(int i=0;i<siz;i++) data[i].~T();
		::operator delete[] (data);

		siz=other.size();
		real_siz=std::max<int>(initial_size,siz);
		data=static_cast<T*>(::operator new[](real_siz*sizeof(T)));
		for(int i=0;i<other.size();i++) new (data+i) T(other[i]);
		return *this;
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 */
	      T & at(const size_t &pos)       {/*if(pos<0||pos>=siz) throw index_out_of_bound();*/assert(pos>=0&&pos<siz);return data[pos];}
	const T & at(const size_t &pos) const {/*if(pos<0||pos>=siz) throw index_out_of_bound();*/assert(pos>=0&&pos<siz);return data[pos];}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 * !!! Pay attentions
	 *   In STL this operator does not check the boundary but I want you to do.
	 */
	      T & operator[](const size_t &pos)       {/*if(pos<0||pos>=siz) throw index_out_of_bound();*/assert(pos>=0&&pos<siz);return data[pos];}
	const T & operator[](const size_t &pos) const {/*if(pos<0||pos>=siz) throw index_out_of_bound();*/assert(pos>=0&&pos<siz);return data[pos];}

	const T & front() const {if(siz==0) throw container_is_empty();return data[0    ];}
	const T & back () const {if(siz==0) throw container_is_empty();return data[siz-1];}
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
	void clear()
	{
		for(int i=0;i<siz;i++) data[i].~T();
		siz=0;
	}
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

		// std::cout<<"Inserting at "<<ind<<std::endl;
		// if((int)ind>siz) throw index_out_of_bound();
		assert(ind<=siz);
		expendOne();
		// std::cout<<"Expended : siz = "<<siz<<std::endl;
		for(int i=siz-2;i>=(int)ind;i--)
		{
			// std::cout<<i<<" "<<ind<<std::endl;
			if(i==siz-2) new (data+i+1) T(data[i]);
			else data[i+1]=data[i];
		}
		// std::cout<<"Moved"<<std::endl;
		if(ind==siz-1) new (data+ind) T(value);
		else data[ind]=value;
		// std::cout<<"Done"<<std::endl;
		return iterator(this,data+ind);
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
		// if((int)ind>=siz) throw index_out_of_bound();
		assert(ind<siz);
		for(int i=ind;i<siz-1;i++) data[i]=data[i+1];
		data[siz-1].~T();
		siz--;
		return iterator(this,data+ind);
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

template<typename T>
const int vector<T>::initial_size=4;
template<typename T>
const int vector<T>::scaleup_rate=2;

#endif

}  // namespace sjtu

#endif