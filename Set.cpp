#include <cstddef>
#include <initializer_list>

template<class T>
class Set {
public:
    struct Node
    {
        T key;
        T max_value;
        unsigned char height;
        Node* left;
        Node* right;
        Node* anc;

        explicit Node(T k) {
            key = k;
            max_value = k;
            left = right = nullptr;
            height = 1;
        }
    };

    class iterator {
    public:
        iterator() {
            node = nullptr;
            is_end = false;
        }

        iterator(Node* node_, bool is_end_) {
            node = node_;
            is_end = is_end_;
        }

        T& operator* () {
            return node->key;
        }

        const T* operator ->() {
            return &(node->key);
        }

        iterator& operator ++() {
            Node* prev_node_ = node;
            node = next_node(node);
            if (node == nullptr) {
                node = prev_node_;
                is_end = true;
            }
            return *this;
        }

        iterator& operator --() {
            if (is_end) {
                is_end = false;
                return *this;
            }
            node = prev_node(node);
            return *this;
        }

        iterator operator ++(int) {
            iterator now(node, is_end);
            Node* prev_node_ = node;
            node = next_node(node);
            if (node == nullptr) {
                node = prev_node_;
                is_end = true;
            }
            return now;
        }

        iterator operator --(int) {
            iterator now(node, is_end);
            if (is_end) {
                is_end = false;
                return now;
            }
            node = prev_node(node);
            return now;
        }

        bool operator ==(iterator other) const {
            return (is_end == other.is_end) && (node == other.node);
        }

        bool operator != (iterator other) const {
            return (is_end != other.is_end) || (node != other.node);
        }

        Node* next_node(Node* now) {
            Node* p = nullptr;
            while (now != nullptr && now->right == p) {
                p = now;
                now = now->anc;
            }
            if (now == nullptr) {
                return nullptr;
            }
            if (p != nullptr && now->left == p) {
                return now;
            }
            now = now->right;
            while (now->left != nullptr) {
                now = now->left;
            }
            return now;
        }

        Node* prev_node(Node* now) {
            Node* p = nullptr;
            while (now != nullptr && now->left == p) {
                p = now;
                now = now->anc;
            }
            if (now == nullptr) {
                return nullptr;
            }
            if (p != nullptr && now->right == p) {
                return now;
            }
            now = now->left;
            while (now->right != nullptr) {
                now = now->right;
            }
            return now;
        }

        Node* node;
        bool is_end;
    };

    Set() {}

    template<typename Iterator>
    Set(Iterator first, Iterator last) {
        while (first != last) {
            root = insert(root, *first);
            ++first;
        }
    }

    Set(std::initializer_list<T> elems) {
        for (T elem : elems) {
            root = insert(root, elem);
        }
    }

    Set(const Set<T>& set) {
        for (T elem : set) {
            insert(elem);
        }
    }

    ~Set() {
        delete_nodes(root);
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void insert(const T& elem) {
        root = insert(root, elem);
    }

    void erase(const T& elem) {
        root = remove(root, elem);
    }

    iterator begin() const {
        if (root == nullptr) {
            return end();
        }
        iterator it(find_min(root), false);
        return it;
    }

    iterator end() const {
        if (root == nullptr) {
            iterator it(nullptr, true);
            return it;
        }
        iterator it(find_max(root), true);
        return it;
    }

    iterator find(const T& elem) const {
        if (root == nullptr) {
            return end();
        }
        Node* node = find_node(root, elem);
        if (node == nullptr) {
            return end();
        }
        iterator it(node, false);
        return it;
    }

    iterator lower_bound(const T& elem) const {
        if (root == nullptr) {
            return end();
        }
        Node* node = lower_bound_node(root, elem);
        if (node == nullptr) {
            return end();
        }
        iterator it(node, false);
        return it;
    }

    Set& operator =(const Set<T>& set) {
        if (&set == this) {
            return *this;
        }
        delete_nodes(root);
        root = nullptr;
        size_ = 0;
        for (T elem : set) {
            insert(elem);
        }
        return *this;
    }

private:

    unsigned char get_height(Node* node)
    {
        if (node == nullptr) {
            return 0;
        }
        return node->height;
    }

    int balance_factor(Node* node)
    {
        return get_height(node->right) - get_height(node->left);
    }

    void update(Node* node)
    {
        unsigned char height_left = get_height(node->left);
        unsigned char height_right = get_height(node->right);
        node->height = height_left;
        if (height_right > node->height) {
            node->height = height_right;
        }
        ++node->height;
        node->max_value = node->key;
        if (node->right != nullptr) {
            node->max_value = node->right->max_value;
        }
        node->anc = nullptr;
        if (node->left != nullptr) {
            node->left->anc = node;
        }
        if (node->right != nullptr) {
            node->right->anc = node;
        }
    }

    Node* rotate_right(Node* node)
    {
        Node* son = node->left;
        node->left = son->right;
        son->right = node;
        update(node);
        update(son);
        return son;
    }

    Node* rotate_left(Node* node)
    {
        Node* son = node->right;
        node->right = son->left;
        son->left = node;
        update(node);
        update(son);
        return son;
    }

    Node* balance(Node* node)
    {
        update(node);
        if (balance_factor(node) == 2) {
            if (balance_factor(node->right) < 0) {
                node->right = rotate_right(node->right);
            }
            return rotate_left(node);
        }
        if (balance_factor(node) == -2)
        {
            if (balance_factor(node->left) > 0) {
                node->left = rotate_left(node->left);
            }
            return rotate_right(node);
        }
        return node;
    }

    Node* insert(Node* node, T k)
    {
        if (!node) {
            ++size_;
            return new Node(k);
        }
        if (k < node->key) {
            node->left = insert(node->left, k);
        }
        else if (node->key < k) {
            node->right = insert(node->right, k);
        }
        else {
            return node;
        }
        return balance(node);
    }

    Node* find_min(Node* node) const
    {
        if (node->left == nullptr) {
            return node;
        }
        return find_min(node->left);
    }

    Node* find_max(Node* node) const
    {
        if (node->right == nullptr) {
            return node;
        }
        return find_max(node->right);
    }

    Node* remove_min(Node* node)
    {
        if (node->left == 0) {
            return node->right;
        }
        node->left = remove_min(node->left);
        return balance(node);
    }

    Node* remove(Node* node, T k)
    {
        if (node == nullptr) {
            return node;
        }
        if (k < node->key) {
            node->left = remove(node->left, k);
        }
        else if (node->key < k) {
            node->right = remove(node->right, k);
        }
        else {
            Node* q = node->left;
            Node* r = node->right;
            --size_;
            delete node;
            if( !r ) return q;
            Node* min = find_min(r);
            min->right = remove_min(r);
            min->left = q;
            return balance(min);
        }
        return balance(node);
    }

    Node* find_node(Node* node, T key) const {
        if (node == nullptr) {
            return node;
        }
        if (key < node->key) {
             return find_node(node->left, key);
        }
        else if (node->key < key) {
            return find_node(node->right, key);
        }
        return node;
    }

    Node* lower_bound_node(Node* node, T key) const {
        if (node == nullptr) {
            return node;
        }
        if (node->key < key) {
            return lower_bound_node(node->right, key);
        }
        if (node->left == nullptr) {
            return node;
        }
        if (node->left->max_value < key) {
            return node;
        }
        return lower_bound_node(node->left, key);
    }

    void delete_nodes(Node* node) {
        if (node == nullptr) {
            return;
        }
        delete_nodes(node->left);
        delete_nodes(node->right);
        delete node;
    }

    Node* root = nullptr;
    size_t size_ = 0;
};
