#include <iostream>
#include <string>
using namespace std;

class Student {
public:
    string name;
    string id;
    string gender;
    string course;

    void input() {
        cout << "\nEnter Student Details" << endl;
        cout << "Name   : ";
        cin >> name;

        cout << "ID     : ";
        cin >> id;

        cout << "Gender : ";
        cin >> gender;

        cout << "Course (BIT/BCS/DIT): ";
        cin >> course;
    }

    void display() {
        cout << "\nStudent Details" << endl;
        cout << "Name   : " << name << endl;
        cout << "ID     : " << id << endl;
        cout << "Gender : " << gender << endl;
        cout << "Course : " << course << endl;
        cout << "------------------------" << endl;
    }
};

int main() {

    Student student[5];

    // Input details for 5 students
    for (int i = 0; i < 5; i++) {
        cout << "\n===== Student " << i + 1 << " =====" << endl;
        student[i].input();
    }

    // Display all students
    cout << "\n===== Student Information =====" << endl;

    for (int i = 0; i < 5; i++) {
        student[i].display();
    }

    return 0;
}