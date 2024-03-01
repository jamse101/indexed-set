#include <iostream>
#include <cstdint>
#include <set>
#include <chrono>
#include <string>
#include <random>

/* Red-black tree algorithm from Robert Sedgewick's Algorithms in C (1990)
   augmented with sizes of left subtrees for order statistic tree (rank and
   select) implementation.

   Public domain by Janne Heikkinen,February 2024 */

namespace jh {

struct rb_tree_rs {
    uint32_t red : 1;
    uint32_t size : 31;
};

template<typename T>
struct rb_node {
    T key;
    rb_tree_rs rs;
    rb_node *l, *r;
};

template<typename T>
class rb_tree {

    rb_node<T> *head{nullptr}, *z{nullptr}, *p{nullptr},
               *g{nullptr}, *gg{nullptr}, *x{nullptr};

    size_t sz{0};

    T not_found{};

    int r_max{0}; // Maximum recursion depth in inorder traversal for debugging.

    rb_node<T> *rotate(T v, rb_node<T> *y)
    {
        rb_node<T> *c, *gc;

        c = (v < y->key) ? y->l : y->r;

        if (v < c->key)
        {
            gc = c->l;
            c->l = gc->r;
            gc->r = c;
            c->rs.size = c->rs.size - gc->rs.size - 1;
        }
        else
        {
            gc = c->r;
            c->r = gc->l;
            gc->l = c;
            gc->rs.size = c->rs.size + gc->rs.size + 1;
        }

        if (v < y->key)
            y->l = gc;
        else
            y->r = gc;

        return gc;
    }

    void split(T v)
    {
        x->rs.red = 1; x->l->rs.red = 0; x->r->rs.red = 0;

        if (p->rs.red)
        {
            g->rs.red = 1;

            if (v < g->key != v < p->key)
                p = rotate(v, g);

            x = rotate(v, gg);

            x->rs.red = 0;
        }

        head->r->rs.red = 0;
    }

    /* When nodes contain size of left subtrees, it is only necessary to update
       size on path from root to newly inserted node when left turn is taken on
       the path. */

    void fix_path(T v)
    {
        rb_node<T> *n = head->r;

        while (n != z)
        {
            if (v == n->key)
                return;

            if (v < n->key)
            {
                n->rs.size++;
                n = n->l;
            }
            else
                n = n->r;
        }
    }

    void inorder_helper(rb_node<T> *n, int d, bool print)
    {
        if (d > r_max)
            r_max = d;

        if (n->l != z)
            inorder_helper(n->l, d+1, print);

        if (print)
            std::cout << n->key << " ";

        if (n->r != z)
            inorder_helper(n->r, d+1, print);
    }


public:

    rb_tree(T nf = T{}) : not_found{nf}
    {
        z = new rb_node<T>{};
        z->l = z->r = z;
        z->rs.red = 0;
        z->rs.size = 0;

        head = new rb_node<T>{};
        head->l = head->r = z;
        head->key = not_found;
        head->rs.red = 0;
        head->rs.size = 0;
    }

    bool insert(T v)
    {
        x = p = g = head;

        while (x != z)
        {
            gg = g; g = p; p = x;

            if (x != head && v == x->key)
                return false;

            x = (x != head && v < x->key) ? x->l : x->r;

            if (x->l->rs.red && x->r->rs.red)
                split(v);
        }

        x = new rb_node<T>{};
        x->key = v;
        x->l = x->r = z;
        x->rs.size = 0;

        sz++;

        if (p != head && v < p->key)
            p->l = x;
        else
            p->r = x;

        fix_path(v);

        split(v);

        return true;
    }

    int rank(T v) const
    {
        rb_node<T> *n = head->r;

        int s = 0;

        while (n != z)
        {
            if (v == n->key)
                return s + n->rs.size;

            if (v < n->key)
                n = n->l;
            else
            {
                s += n->rs.size + 1;
                n = n->r;
            }
        }

        return -1;
    }

    T select(int ndx) const
    {
        rb_node<T> *n = head->r;

        while (n != z)
        {
            int s = n->rs.size;

            if (ndx == s)
                return n->key;
            else if (ndx < s)
                n = n->l;
            else
            {
                n = n->r;
                ndx = ndx - s - 1;
            }
        }

        return not_found; // not possible if ndx is in range
    }

    size_t size() const
    {
        return sz;
    }

    void inorder(bool print)
    {
        if (print)
            std::cout << "inorder:\n";

        inorder_helper(head->r, 0, print);

        if (print)
            std::cout << "\nend\n";
    }

    int rmax() const
    {
        return r_max;
    }
};

}

int main()
{
    jh::rb_tree<int> rb(-1);
    std::set<int> s;

    int N1 = 200'000;
    int N2 = 10'000'000; // big enough to include every number in range (0,N1-1)

    std::mt19937 gen(1);
    std::uniform_int_distribution<int>  dist(0, N1-1);

    auto t0 = std::chrono::steady_clock::now();

    for (int i = 0 ; i < N2 ; i++)
    {
        int v = dist(gen);

        rb.insert(v);
    }

    auto t1 = std::chrono::steady_clock::now();

    for (int i = 0 ; i < N1 ; i++)
        if(i != rb.rank(i))
            std::cout << i << " != " << rb.rank(i) << "\n";

    for (int i = 0 ; i < N1 ; i++)
        if(i != rb.select(i))
            std::cout << i << " != " << rb.select(i) << "\n";

    gen.seed(1);

    for (int i = 0  ; i < N2 ; i++)
    {
        int v = dist(gen);

        s.insert(v);
    }

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "rb.size(): " << rb.size() << " s.size(): " << s.size() << "\n";

    std::chrono::duration<double> dur1 = t1 - t0;
    std::chrono::duration<double> dur2 = t2 - t1;

    std::cout << dur1.count() << " " << dur2.count() << "\n";

    rb.inorder(false);

    std::cout << "r_max: " << rb.rmax() << "\n";

    jh::rb_tree<std::string> rbs("not found");

    rbs.insert("World!");
    rbs.insert("Hello,");

    rbs.inorder(true);

    std::cout << rbs.select(3) << "\n"; // "not found"

    return 0;
}
