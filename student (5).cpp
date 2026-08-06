#include <iostream>
#include <string>
using namespace std;

class student{
        public:
                string Name;
                string StudentID;
                string Course;

                void studentDetails(){
                        cout << "Name: " << Name << endl;
                        cout << "Student ID: " << StudentID << endl;
                        cout << "Course: " << Course << endl;
                }
};

int main(){
        student student1;
        cout << "--- Student 1 ---" << endl;
        student1. Name = "Len Pei Ying";
        student1. StudentID = "QIU-202404-007159";
        student1. Course = "BIT";
        student1. studentDetails();
        cout << endl;

        student student2;
        cout << "--- Student 2 ---" << endl;
        student2. Name = "Chan Jade Qi";
        student2. StudentID = "QIU-202404-007178";
        student2. Course = "BIT";
        student2. studentDetails();
        cout << endl;

        student student3;
        cout << "--- Student 3 ---" <<endl;
        student3. Name = "Lim Jie Yee";
        student3. StudentID = "QIU-202407-007473";
        student3. Course = "BIT";
        student3. studentDetails();
        cout << endl;

        student student4;
        cout << "--- Student 4 ---" << endl;
        student4. Name = "Woo An Qi";
        student4. StudentID = "QIU-202404-007178";
        student4. Course = "BIT";
        student4. studentDetails();
        cout << endl;

        student student5;
        cout << "--- Student 5 ---" << endl;
        student5. Name = "Wong Ke Ni";
        student5. StudentID = "QIU-202310-006100";
        student5. Course = "BCC";
        student5. studentDetails();
        cout << endl;

}
