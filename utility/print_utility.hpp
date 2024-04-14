#include <iostream>
#include "../trie/src2.hpp"
#include <vector>
#include <string>
#include <queue>
using std::cout, std::endl, std::queue;

//Feel free to design your own print function for testing / debugging purpose
// void print_trie(std::shared_ptr<sjtu::TrieNode> trienode, std::vector<std::string> path);

void print_trie(std::shared_ptr<sjtu::TrieNode> root){
    if(!root){
        cout<<"tree is empty!"<<endl;
        return;
    }
    queue< std::shared_ptr<sjtu::TrieNode> > q;
    q.push(root);
    while(!q.empty()){
        auto now = q.front();
        q.pop();
        cout<<"id = "<<now->node_id<<", ";
        if(now->is_value_node_){
            // std::shared_ptr< sjtu::TrieNodeWithValue<T> > des = std::dynamic_pointer_cast< sjtu::TrieNodeWithValue<T> >(now);
            // cout<<des->value_<<endl;
            cout<<"has value!"<<endl;
        }
        else cout<<"no value!"<<endl;
        for(auto it = now->children_.begin(); it!=now->children_.end();it++){
            cout<<it->first<<" to "<<it->second->node_id<<endl;
            q.push(it->second);
        }
        cout<<"---------------"<<endl;
    }
};