#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>

uint8_t countParams(std::string path);


typedef uint8_t node_t;

typedef std::function<void(void)> handler_t;

enum httprouter:node_t{
  static_ = 0,
  root_,
  param_,
  catchAll_
};

typedef struct _tree_node{
  std::string                     path;
  bool                            wildChild;
  node_t                          nType;
  uint8_t                         maxParams;
  std::string                     indices;
  std::vector<struct _tree_node*> children;
  handler_t                       handle;  
  uint32_t                        priority;
}tree_node_t;

class node 
{
public:
  node(tree_node_t* node);
  //! addRouter adds a node with the given handle to the path
  //! Not concurrency-safe!
  void addRoute(std::string path, handler_t handle);

  void insertChild(tree_node_t* node, uint8_t numParams, std::string path, std::string fullPath, handler_t handle);

  handler_t getValue(std::string path);
private:
  tree_node_t* node_;
};

node::node(tree_node_t* node)
  :node_(node)
{
}

void node::addRoute(std::string path, handler_t handle){
  std::string fullPath = path;
  node_->priority++;
  auto numParams = countParams(path);
  auto node = node_;

  // no-empty tree
  if(node->path.size() > 0 || node->children.size() > 0){
    while(true){
      bool have_indices =false;
      // update maxParams of the current node.
      if(numParams > node->maxParams) {
        node->maxParams = numParams;
      }

      //find the longest common prefix.
      auto i = 0;
      auto max = std::min(node->path.size(), path.size());
      for(;i < max && path[i] == node->path[i];){
        i++;
      }

      // Split edge
      if( i < node->path.size()){
        auto child = new tree_node_t{
          path:       std::string(node->path.c_str() + i),
          wildChild:  node->wildChild,
          nType:      static_,
          maxParams:  0,
          indices:    node->indices,
          children:   std::move(node->children),
          handle:     node->handle,
          priority:   node->priority - 1
        };

        // Update maxParams (max of all children)
        for(auto it = child->children.begin(); it != child->children.end(); it++){
          if((*it)->maxParams > child->maxParams) {
            child->maxParams = (*it)->maxParams;
          }
        }

        node->children = std::vector<tree_node_t*>{child};
        node->indices = std::string(node->path.c_str() + i, 1);
        node->path = std::string(path.c_str(), i);
        node->handle = nullptr;
        node->wildChild = false;
      }

      // make new node a child of this node 
      if(i < path.size()){
        path = std::string(path.c_str() + i);

        if(node->wildChild) {
          std::cout << "tag" << std::endl;
        }

        char ch = path[0];
        
        //slash after param 
        if(node->nType == param_ && ch == '/' && node->children.size() == 1){
          node = node->children[0];
          node->priority++;
          continue;
        }

        // Check if a child with the next path byte exists
        for(int i =0; i < node->indices.size(); i++){
          if( ch == node->indices[i]) {
            //i = node.incrementChildPrio(i);
            node = node->children[i];
            have_indices = true;
          }
        }
        if(have_indices){
          continue;
        }

        //otherwise insert it 
        if(ch != ':' && ch != '*') {
          node->indices += ch;
          auto child = new tree_node_t{
          path:       "",
          wildChild:  false,
          nType:      static_,
          maxParams:  numParams,
          indices:    "",
          children:   {},
          handle:     nullptr,
          priority:   0
        };
          node->children.push_back(child);
          //node->incrementChildPrio();
          node = child;
        }
        insertChild(node, numParams, path, fullPath, handle);
        return ;
      } else if (i == path.size()){
        if(node->handle){
          std::cout << "error ! handle already exists.";
        }
        node->handle = handle;
      }
      return;
    }  
  } else { // Empty tree
    insertChild(node, numParams, path, fullPath, handle);
    node->nType = root_;
  }

}

void node::insertChild(tree_node_t* node, uint8_t numParams, std::string path, std::string fullPath, handler_t handle){
  int offset = 0;

  int i = 0;
  std::size_t max = path.size();

  for(; numParams > 0; i++){
    char ch = path[i];
    if(ch != ':' && ch != '*'){
      continue;
    }
    //fix me;
  }

  node->path = std::string(path.c_str() + offset);
  node->handle = handle;
}


handler_t node::getValue(std::string path){
  auto node = node_;
  while(true){
    if(path.size() > node->path.size()){
      if(std::string(path.c_str(), node->path.size()) == node->path){
        path = std::string(path.c_str() + node->path.size()); 
      }

      if (!node->wildChild) {
        char ch = path[0];
        for(int i = 0; i < node->indices.size(); i++){
          if (ch == node->indices[i]) {
            node = node->children[i];
            continue;
          }
        }
      }

    // handle wildcard child
    // fix me
    } else if (path == node->path) {
      return node->handle;
    }
  }
}

//! addRouter adds a node with the given handle to the path
//! Not concurrency-safe!

void hello1(){
  std::cout << "hello1" << std::endl;
}
void hello2(){
  std::cout << "hello2" << std::endl;
}
void hello3(){
  std::cout << "hello3" << std::endl;
}
void hello4(){
  std::cout << "hello4" << std::endl;
}

int main() {
	std::string test = "/serach/";

  tree_node_t* n = new tree_node_t;

  node no(n);

  no.addRoute("/", hello1);

  no.addRoute("/serach/", hello2);

  no.addRoute("/support/", hello3);

  no.addRoute("/blog/", hello4);

  no.getValue("/")();
  no.getValue("/serach/")();
  no.getValue("/support/")();
  no.getValue("/blog/")();

  std::cout << static_ << catchAll_ << sizeof(catchAll_) << std::endl;

	std::cout << countParams(test) << std::endl;

  while(1);

}



//! count the : and * params counts.
uint8_t countParams(std::string path) {
	uint8_t n = 0;
	for (int i = 0; i < path.size(); i++) {
		if (path[i] != ':' && path[i] != '*') {
			continue;
		}
		n++;
	}
	if (n >= 255) {
		return 255;
	}
	return n;
}
