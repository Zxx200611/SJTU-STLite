/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

#include<cassert>
// #include<iostream>

namespace sjtu
{

template<class Key,class T,class Compare = std::less <Key>> class map
{
public:
    typedef pair<const Key, T> value_type;
private:
    struct ExValueType
    {
        bool is_infty;
        value_type *vt;

        ExValueType(bool _is_infty,value_type *_vt):is_infty(_is_infty),vt(_vt){}
        bool operator < (const ExValueType &b) const
        {
            if(is_infty!=b.is_infty) return is_infty<b.is_infty;
            return Compare()(vt->first,b.vt->first);
        }
        bool operator < (const Key &k) const
        {
            if(is_infty) return 0;
            return Compare()(vt->first,k);
        }
        bool operator > (const Key &k) const
        {
            if(is_infty) return 1;
            return Compare()(k,vt->first);
        }
        bool operator == (const Key &k) const
        {
            if(is_infty) return 0;
            return !Compare()(vt->first,k)&&!Compare()(k,vt->first);
        }
        bool operator != (const Key &k) const
        {
            return !operator==(k);
        }
    };
    struct Node
    {
        ExValueType val;
        Node *fa,*ch[2];
        bool is_red;

        Node():val(1,nullptr),fa(nullptr),is_red(0){ch[0]=ch[1]=nullptr;}   // default is infty
        Node(const Node &v):val(v.val.is_infty,v.val.is_infty?nullptr:new value_type(*v.val.vt)),
                           fa(v.fa),is_red(v.is_red){ch[0]=v.ch[0],ch[1]=v.ch[1];}
        Node(const Key &_key,const T &_val,Node *_fa):val(0,new value_type(_key,_val)),fa(_fa),is_red(1){ch[0]=ch[1]=nullptr;}
        bool dir() const
        {
            if(fa==nullptr) return 0;
            return fa->ch[1]==this;
        }
        ~Node(){delete val.vt;}
    };
    Node *rt,*mnp,*mxp;
    int siz;

    
    void copyTree(Node *&u,Node *v,Node *ufa)
    {
        // std::cout<<"Copying "<<v<<std::endl;
        if(v==nullptr)
        {
            u=nullptr;
            return;
        }

        u=new Node(*v),u->fa=ufa;
        copyTree(u->ch[0],v->ch[0],u);
        copyTree(u->ch[1],v->ch[1],u);
        if(mnp==nullptr||u->val<mnp->val) mnp=u;
        if(mxp==nullptr||mxp->val<u->val) mxp=u;
    }
    void deleteTree(Node *u)
    {
        if(u==nullptr) return;
        deleteTree(u->ch[0]),deleteTree(u->ch[1]);
        delete u;
    }
    void rotate(Node *u)
    {
        assert(u->fa!=nullptr);
        Node *v=u->fa,*w=(v!=nullptr&&v->fa!=nullptr?v->fa:nullptr);
        bool d=u->dir(),dv=(w==nullptr?0:v->dir());

        v->ch[d]=u->ch[d^1];
        u->ch[d^1]=v;

        if(v->ch[d]!=nullptr) v->ch[d]->fa=v;
        v->fa=u;
        u->fa=w;

        if(w!=nullptr) w->ch[dv]=u;
        else rt=u;
    }
    Node *lowerBound(const Key &k) const
    {
        Node *u=rt,*res=u;
        while(u!=nullptr)
        {
            bool d1=(u->val<k),d2=(u->val>k);
            if(!d1&&!d2) return u;
            if(d2) res=u;
            u=u->ch[d1];
        }
        return res;
    }
    bool isRed(Node *u)
    {
        if(u==nullptr) return 0;
        return u->is_red;
    }
    pair<Node*,bool> insert(const Key &k,const T &v)
    {
        Node *n=rt,*p=nullptr;
        while(n!=nullptr)
        {
            p=n;
            bool d0=(p->val<k),d1=(p->val>k);
            if(d0==0&&d1==0) return pair<Node*,bool>(n,0);
            n=n->ch[d0];
        }
        assert(p!=nullptr);
        n=new Node(k,v,p);
        p->ch[p->val<k]=n;
        Node *res=n;

        while(isRed(p=n->fa))
        {
            Node *g=p->fa,*u=g->ch[!p->dir()];
            if(isRed(u))
            {
                g->is_red^=1,p->is_red^=1,u->is_red^=1;
                n=g;
            }
            else
            {
                if(p->dir()!=n->dir()) rotate(n),std::swap(n,p);
                rotate(p);
                std::swap(p->is_red,g->is_red);
                break;
            }
        }
        rt->is_red=0;
        if(mnp==nullptr||res->val<mnp->val) mnp=res;
        if(mxp==nullptr||mxp->val<res->val) mxp=res;
        siz++;
        return pair<Node*,bool>(res,1);
    }
    Node *minNodeUnder(Node *u)
    {
        while(u->ch[0]!=nullptr) u=u->ch[0];
        return u;
    }
    Node *maxNodeUnder(Node *u)
    {
        while(u->ch[1]!=nullptr) u=u->ch[1];
        return u;
    }
    void remove(Node *u)
    {
        if(u==mnp) mnp=findNext(u);
        if(u==mxp) mxp=findPrev(u);

        if(u->ch[0]!=nullptr&&u->ch[1]!=nullptr)
        {
            Node *v=minNodeUnder(u->ch[1]);

            if(v->fa==u)
            {
                Node *fu=u->fa;
                bool du=u->dir();
                
                if(u->ch[0]!=nullptr) u->ch[0]->fa=v;
                v->ch[0]=u->ch[0];
                u->ch[0]=nullptr;

                if(v->ch[1]!=nullptr) v->ch[1]->fa=u;
                u->ch[1]=v->ch[1];
                v->ch[1]=u,u->fa=v;

                if(fu==nullptr) rt=v;
                else fu->ch[du]=v;
                v->fa=fu;
            }
            else
            {
                Node *fu=u->fa,*fv=v->fa;
                bool du=u->dir(),dv=v->dir();
                if(u->ch[0]) u->ch[0]->fa=v;
                if(u->ch[1]) u->ch[1]->fa=v;
                if(v->ch[0]) v->ch[0]->fa=u;
                if(v->ch[1]) v->ch[1]->fa=u;
                std::swap(u->ch[0],v->ch[0]);
                std::swap(u->ch[1],v->ch[1]);

                u->fa=fv,v->fa=fu;
                if(fu==nullptr) rt=v;
                else fu->ch[du]=v;
                fv->ch[dv]=u;
            }
            std::swap(u->is_red,v->is_red);
        }
        if(u->ch[0]!=nullptr||u->ch[1]!=nullptr)
        {
            bool d=(u->ch[1]!=nullptr);
            if(u==rt) rt=u->ch[d],rt->fa=nullptr,rt->is_red=0;
            else
            {
                u->fa->ch[u->dir()]=u->ch[d],u->ch[d]->fa=u->fa;
                u->ch[d]->is_red=0;
            }
            delete u;
        }
        else
        {
            if(isRed(u)==0)
            {
                for(Node *n=u,*p=n->fa;p!=nullptr&&isRed(n)==0;n=p,p=n->fa)
                {
                    bool n_dir=n->dir();
                    Node *s=p->ch[n_dir^1];
                    if(s==nullptr) break;

                    if(isRed(s))
                    {
                        rotate(s);
                        std::swap(p->is_red,s->is_red);
                        s=p->ch[n_dir^1];
                    }
                    Node *c=s->ch[n_dir],*d=s->ch[n_dir^1];
                    if(!isRed(s)&&!isRed(c)&&!isRed(d))
                    {
                        s->is_red=1;
                        if(isRed(p))
                        {
                            p->is_red=0;
                            break;
                        }
                    }
                    else
                    {
                        if(!isRed(s)&&isRed(c)&&!isRed(d))
                        {
                            rotate(c);
                            std::swap(c->is_red,s->is_red);
                            s=c;
                            c=s->ch[n_dir],d=s->ch[n_dir^1];
                        }
                        if(!isRed(s)&&isRed(d))
                        {
                            rotate(s);
                            std::swap(p->is_red,s->is_red);
                            d->is_red=0;
                            break;
                        }
                    }
                }
            }
            assert(u->fa!=nullptr);
            u->fa->ch[u->dir()]=nullptr;
            delete u;
        }

        siz--;
        return;
    }
    Node *findNext(Node *u)
    {
        if(u->ch[1])
        {
            u=u->ch[1];
            while(u->ch[0]!=nullptr) u=u->ch[0];
            return u;
        }
        while(u->fa!=nullptr&&u->dir()==1) u=u->fa;
        return u->fa;
    }
    Node *findPrev(Node *u)
    {
        if(u->ch[0])
        {
            u=u->ch[0];
            while(u->ch[1]!=nullptr) u=u->ch[1];
            return u;
        }
        while(u->fa!=nullptr&&u->dir()==0) u=u->fa;
        return u->fa;
    }
    const Node *findNext(const Node *u) const
    {
        if(u->ch[1])
        {
            u=u->ch[1];
            while(u->ch[0]!=nullptr) u=u->ch[0];
            return u;
        }
        while(u->fa!=nullptr&&u->dir()==1) u=u->fa;
        return u->fa;
    }
    const Node *findPrev(const Node *u) const
    {
        if(u->ch[0])
        {
            u=u->ch[0];
            while(u->ch[1]!=nullptr) u=u->ch[1];
            return u;
        }
        while(u->fa!=nullptr&&u->dir()==0) u=u->fa;
        return u->fa;
    }

public:
    /**
     * see BidirectionalIterator at CppReference for help.
     *
     * if there is anything wrong throw invalid_iterator.
     *     like it = map.begin(); --it;
     *       or it = map.end(); ++end();
     */
    class const_iterator;
    class iterator
    {
        friend class map;
    private:
        map *bel;
        Node *u;
        iterator(map *_bel,Node *_u):bel(_bel),u(_u){}
            
    public:
        iterator():bel(nullptr),u(nullptr){}
        iterator(const iterator &other):bel(other.bel),u(other.u){}

        iterator operator++(int)
        {
            Node *tmp=bel->findNext(u);
            if(tmp==nullptr) throw invalid_iterator();
            std::swap(u,tmp);
            return iterator(bel,tmp);
        }
        iterator &operator++()
        {
            Node *tmp=bel->findNext(u);
            if(tmp==nullptr) throw invalid_iterator();
            u=tmp;
            return *this;
        }
        iterator operator--(int)
        {
            Node *tmp=bel->findPrev(u);
            if(tmp==nullptr) throw invalid_iterator();
            std::swap(u,tmp);
            return iterator(bel,tmp);
        }
        iterator &operator--()
        {
            Node *tmp=bel->findPrev(u);
            if(tmp==nullptr) throw invalid_iterator();
            u=tmp;
            return *this;
        }

        value_type &operator*() const
        {
            if(u==nullptr||u->val.is_infty) throw invalid_iterator();
            return *u->val.vt;
        }

        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const {return u==rhs.u;}
        bool operator==(const const_iterator &rhs) const {return u==rhs.u;}
        bool operator!=(const iterator &rhs) const {return u!=rhs.u;}
        bool operator!=(const const_iterator &rhs) const {return u!=rhs.u;}

        /**
         * for the support of it->first.
         * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
         */
        value_type *operator->() const
        {
            if(u==nullptr||u->val.is_infty) throw invalid_iterator();
            return u->val.vt;
        }
    };
    class const_iterator
    {
        // it should has similar member method as iterator.
        //  and it should be able to construct from an iterator.
        friend class map;
    private:
        const map *bel;
        const Node *u;
        const_iterator(map *_bel,Node *_u):bel(_bel),u(_u){}
        const_iterator(const map *_bel,const Node *_u):bel(_bel),u(_u){}
        
    public:
        const_iterator():bel(nullptr),u(nullptr){}
        const_iterator(const iterator &other):bel(other.bel),u(other.u){}
        const_iterator(const const_iterator &other):bel(other.bel),u(other.u){}

        const_iterator operator++(int)
        {
            const Node *tmp=bel->findNext(u);
            if(tmp==nullptr) throw invalid_iterator();
            std::swap(u,tmp);
            return const_iterator(bel,tmp);
        }
        const_iterator &operator++()
        {
            const Node *tmp=bel->findNext(u);
            if(tmp==nullptr) throw invalid_iterator();
            u=tmp;
            return *this;
        }
        const_iterator operator--(int)
        {
            const Node *tmp=bel->findPrev(u);
            if(tmp==nullptr) throw invalid_iterator();
            std::swap(u,tmp);
            return const_iterator(bel,tmp);
        }
        const_iterator &operator--()
        {
            const Node *tmp=bel->findPrev(u);
            if(tmp==nullptr) throw invalid_iterator();
            u=tmp;
            return *this;
        }

        const value_type &operator*() const
        {
            if(u==nullptr||u->val.is_infty) throw invalid_iterator();
            return *u->val.vt;
        }

        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const {return u==rhs.u;}
        bool operator==(const const_iterator &rhs) const {return u==rhs.u;}
        bool operator!=(const iterator &rhs) const {return u!=rhs.u;}
        bool operator!=(const const_iterator &rhs) const {return u!=rhs.u;}

        /**
         * for the support of it->first.
         * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
         */
        const value_type *operator->() const
        {
            if(u==nullptr||u->val.is_infty) throw invalid_iterator();
            return u->val.vt;
        }
    };

    map():rt(new Node()),mnp(rt),mxp(rt),siz(0){}
    map(const map &other):rt(nullptr),mnp(nullptr),mxp(nullptr),siz(other.siz)
    {
        copyTree(rt,other.rt,nullptr);
    }
    map &operator=(const map &other)
    {
        if(this==&other) return *this;

        if(rt!=nullptr) deleteTree(rt);
        mnp=nullptr,mxp=nullptr;
        copyTree(rt,other.rt,nullptr);
        siz=other.siz;
        return *this;
    }
    ~map(){deleteTree(rt);}

    /**
     * TODO
     * access specified element with bounds checking
     * Returns a reference to the mapped value of the element with key equivalent to key.
     * If no such element exists, an exception of type `index_out_of_bound'
     */
    T &at(const Key &key)
    {
        Node *x=lowerBound(key);
        if(x->val!=key) throw index_out_of_bound();
        return x->val.vt->second;
    }
    const T &at(const Key &key) const
    {
        Node *x=lowerBound(key);
        if(x->val!=key) throw index_out_of_bound();
        return x->val.vt->second;
    }

    /**
     * TODO
     * access specified element
     * Returns a reference to the value that is mapped to a key equivalent to key,
     *   performing an insertion if such key does not already exist.
     */
    T &operator[](const Key &key)
    {
        Node *x=insert(key,T()).first;
        return x->val.vt->second;
    }

    /**
     * behave like at() throw index_out_of_bound if such key does not exist.
     */
    const T &operator[](const Key &key) const
    {
        return at(key);
    }

    iterator begin() {return iterator(this,mnp);}
    const_iterator cbegin() const {return const_iterator(this,mnp);}
    iterator end() {return iterator(this,mxp);}
    const_iterator cend() const {return const_iterator(this,mxp);}

    /**
     * checks whether the container is empty
     * return true if empty, otherwise false.
     */
    bool empty() const {return mnp==mxp;}

    /**
     * returns the number of elements.
     */
    size_t size() const {return siz;}

    /**
     * clears the contents
     */
    void clear()
    {
        deleteTree(rt);
        mnp=mxp=rt=new Node();
        siz=0;
    }

    /**
     * insert an element.
     * return a pair, the first of the pair is
     *   the iterator to the new element (or the element that prevented the insertion),
     *   the second one is true if insert successfully, or false.
     */
    pair<iterator, bool> insert(const value_type &value)
    {
        pair<Node*,bool> pr=insert(value.first,value.second);
        return pair<iterator,bool>(iterator(this,pr.first),pr.second);
    }

    /**
     * erase the element at pos.
     *
     * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
     */
    void erase(iterator pos)
    {
        if(pos==end()||lowerBound(pos->first)!=pos.u) throw invalid_iterator();
        remove(pos.u);
    }

    /**
     * Returns the number of elements with key
     *   that compares equivalent to the specified argument,
     *   which is either 1 or 0
     *     since this container does not allow duplicates.
     * The default method of check the equivalence is !(a < b || b > a)
     */
    size_t count(const Key &key) const
    {
        Node *p=lowerBound(key);
        if(p->val==key) return 1;
        return 0;
    }

    /**
     * Finds an element with key equivalent to key.
     * key value of the element to search for.
     * Iterator to an element with key equivalent to key.
     *   If no such element is found, past-the-end (see end()) iterator is returned.
     */
    iterator find(const Key &key)
    {
        Node *p=lowerBound(key);
        if(p->val==key) return iterator(this,p);
        return end();
    }
    const_iterator find(const Key &key) const
    {
        Node *p=lowerBound(key);
        if(p->val==key) return const_iterator(this,p);
        return cend();
    }
};

}

#endif
