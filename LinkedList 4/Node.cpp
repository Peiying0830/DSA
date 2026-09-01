// Template class for Node
template <typename T>
class Node {
public:
    T data;          // Data contained in the node
    Node* next;      // Pointer to the next node in the list

    // Constructor to initialize a node
    Node(T value, Node* nextNode = nullptr){
        data = value;
        next = nextNode;
    } 

    // Destructor
    ~Node() {
        next = nullptr; // Nullify the pointer
    }
};
