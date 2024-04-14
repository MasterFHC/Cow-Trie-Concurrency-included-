/*
* Acknowledgement : Yuxuan Wang for Modifying the prototype of TrieStore class
*/

#ifndef SJTU_TRIE_HPP
#define SJTU_TRIE_HPP

#include <algorithm>
#include <cstddef>
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string_view>
#include <iostream>
#include <shared_mutex>
#include <string_view>
#include <mutex>  
#include <future>

namespace sjtu {

    // A TrieNode is a node in a Trie.
    class Trie;
    class TrieNode {
    public:
        // Create a TrieNode with no children.
        TrieNode(){
            node_id = ++node_cnt;
        };

        // Create a TrieNode with some children.
        explicit TrieNode(std::unordered_map<char, std::shared_ptr<TrieNode>> children) : children_(std::move(children)), node_id(++node_cnt) {}

        virtual ~TrieNode() {}

        // Clone returns a copy of this TrieNode. If the TrieNode has a value, the value is copied. The return
        // type of this function is a unique_ptr to a TrieNode.
        //
        // You cannot use the copy constructor to clone the node because it doesn't know whether a `TrieNode`
        // contains a value or not.
        //
        // Note: if you want to convert `unique_ptr` into `shared_ptr`, you can use `std::shared_ptr<T>(std::move(ptr))`.
        virtual auto Clone() const -> std::unique_ptr<TrieNode> { return std::make_unique<TrieNode>(children_); }

        // A map of children, where the key is the next character in the key, and the value is the next TrieNode.
        std::unordered_map<char, std::shared_ptr<TrieNode>> children_;

        // Indicates if the node is the terminal node.
        bool is_value_node_{ false };
        static int node_cnt;
        int node_id;
        // You can add additional fields and methods here. But in general, you don't need to add extra fields to
        // complete this project.
    };
    int TrieNode::node_cnt = 0;
    // A TrieNodeWithValue is a TrieNode that also has a value of type T associated with it.
    template <class T>
    class TrieNodeWithValue : public TrieNode {
    public:
        // Create a trie node with no children and a value.
        explicit TrieNodeWithValue(std::shared_ptr<T> value) : value_(std::move(value)) {
            this->is_value_node_ = true; 
        }

        // Create a trie node with children and a value.
        TrieNodeWithValue(std::unordered_map<char, std::shared_ptr<TrieNode>> children, std::shared_ptr<T> value)
            : TrieNode(std::move(children)), value_(std::move(value)) {
            this->is_value_node_ = true;
        }

        // Override the Clone method to also clone the value.
        //
        // Note: if you want to convert `unique_ptr` into `shared_ptr`, you can use `std::shared_ptr<T>(std::move(ptr))`.
        auto Clone() const -> std::unique_ptr<TrieNode> override {
            if(is_value_node_)
                return std::make_unique<TrieNodeWithValue<T>>(children_, value_);
            else
                return std::make_unique<TrieNode>(children_);
        }

        // The value associated with this trie node.
        std::shared_ptr<T> value_;
    };

    // A Trie is a data structure that maps strings to values of type T. All operations on a Trie should not
    // modify the trie itself. It should reuse the existing nodes as much as possible, and create new nodes to
    // represent the new trie.
    class Trie {
    private:
        // The root of the trie.
        std::shared_ptr<TrieNode> root_{ nullptr };
        // Create a new trie with the given root.
        explicit Trie(std::shared_ptr<TrieNode> root) : root_(std::move(root)) {}

    public:
        // Create an empty trie.
        Trie() = default;

        // Get the value associated with the given key.
        // 1. If the key is not in the trie, return nullptr.
        // 2. If the key is in the trie but the type is mismatched, return nullptr.
        // 3. Otherwise, return the value.
        template <class T>
        auto Get(std::string_view key) const -> T*{
            if(!root_) return nullptr;
            auto now = root_;
            for(int i=0;key[i];i++){
                if(now->children_.find(key[i]) == now->children_.end()){
                    return nullptr;
                }
                now = now->children_[key[i]];
            }
            if(!(now->is_value_node_)) return nullptr;
            std::shared_ptr< TrieNodeWithValue<T> > des = std::dynamic_pointer_cast< TrieNodeWithValue<T> >(now);
            if(typeid(des->value_) != typeid(std::shared_ptr<T>)) return nullptr;
            return des->value_.get();
        }

        // Put a new key-value pair into the trie. If the key already exists, overwrite the value.
        // Returns the new trie.
        template <class T>
        auto Put(std::string_view key, T value) const -> Trie{
            bool on_track = true;
            std::shared_ptr<TrieNode> now, new_root;
            if(root_ == nullptr){
                new_root = std::shared_ptr<TrieNode>(new TrieNode());
            }
            else{
                new_root = std::shared_ptr<TrieNode>(std::move(root_->Clone()));
            }
            now = new_root;
            for(int i=0;key[i+1];i++){
                if(now->children_.find(key[i]) == now->children_.end() or !on_track){
                    on_track = false;
                    now->children_[key[i]] = std::shared_ptr<TrieNode>(new TrieNode());
                }
                else{
                    now->children_[key[i]] = now->children_[key[i]]->Clone();
                }
                now = now->children_[key[i]];
            }
            auto newvalue = std::make_shared<T>(std::move(value));
            
            if(now->children_.find(key[key.size() - 1]) == now->children_.end() or !on_track){
                now->children_[key[key.size() - 1]] = std::shared_ptr<TrieNodeWithValue<T>>(new TrieNodeWithValue<T>(newvalue));
            }
            else{
                now->children_[key[key.size() - 1]] = std::shared_ptr<TrieNodeWithValue<T>>(new TrieNodeWithValue<T>(now->children_[key[key.size() - 1]]->children_, newvalue));
            }
            return Trie(new_root);
        }

        // Remove the key from the trie. If the key does not exist, return the original trie.
        // Otherwise, returns the new trie.
        auto Remove(std::string_view key) const->Trie{
            if(!root_) return *this;
            auto now = root_;
            for(int i=0;key[i];i++){
                if(now->children_.find(key[i]) == now->children_.end()){
                    return *this;
                }
                now = now->children_[key[i]];
            }
            Trie ret(Put<int>(key, -1));
            now = ret.getroot();
            auto important = now;
            char path_char = '\0';
            auto father = now;
            for(int i=0;key[i];i++){
                if(now->children_.size() > 1 or now->is_value_node_){
                    important = now;
                    path_char = key[i];
                }
                father = now;
                now = now->children_[key[i]];
            }
            if(path_char == '\0'){
                ret.root_ = nullptr;
                return ret;
            }
            if(now->children_.empty()){
                important->children_.erase(path_char);
            }
            else{
                now->is_value_node_ = false;
            }
            return ret;
        }
        std::shared_ptr<TrieNode> getroot(){
            return root_;
        }
    };


    // This class is used to guard the value returned by the trie. It holds a reference to the root so
    // that the reference to the value will not be invalidated.
    template <class T>
    class ValueGuard {
    public:
        ValueGuard(Trie root, T& value) : root_(std::move(root)), value_(value) {}
        auto operator*() const -> T& { return value_; }

    private:
        Trie root_;
        T& value_;
    };

    // This class is a thread-safe wrapper around the Trie class. It provides a simple interface for
    // accessing the trie. It should allow concurrent reads and a single write operation at the same
    // time.
    class TrieStore {
    public:
        // This function returns a ValueGuard object that holds a reference to the value in the trie of the given version (default: newest version). If
        // the key does not exist in the trie, it will return std::nullopt.
        template <class T>
        auto Get(std::string_view key, size_t version = -1) -> std::optional<ValueGuard<T>>{
            if(version == -1) version = get_version();
            if(!snapshots_[version].Get<T>(key)) return std::nullopt;
            return ValueGuard<T>(snapshots_[version], *(snapshots_[version].Get<T>(key)));
        }

        // This function will insert the key-value pair into the trie. If the key already exists in the
        // trie, it will overwrite the value
        // return the version number after operation
        // Hint: new version should only be visible after the operation is committed(completed)
        template <class T>
        size_t Put(std::string_view key, T value){
            write_lock_.lock();
            Trie ret = std::move(snapshots_[get_version()].Put<T>(key, std::move(value)));
            snapshots_.push_back(ret);
            write_lock_.unlock();
            return get_version();
        }

        // This function will remove the key-value pair from the trie.
        // return the version number after operation
        // if the key does not exist, version number should not be increased
        size_t Remove(std::string_view key){
            write_lock_.lock();
            Trie ret = std::move(snapshots_[get_version()].Remove(key));
            snapshots_.push_back(ret);
            write_lock_.unlock();
            return get_version();
        }
        Trie getTrie(size_t version = -1){
            if(version == -1) version = get_version();
            return snapshots_[version];
        }
        // This function return the newest version number
        size_t get_version(){
            return snapshots_.size() - 1;
        }

    private:

        // This mutex sequences all writes operations and allows only one write operation at a time.
        // Concurrent modifications should have the effect of applying them in some sequential order
        std::mutex write_lock_;

        // Stores all historical versions of trie
        // version number ranges from [0, snapshots_.size())
        std::vector<Trie> snapshots_{ 1 };
    };

}  // namespace sjtu

#endif  // SJTU_TRIE_HPP