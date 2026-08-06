#include <iostream>
#include <string>
using namespace std;

class Employee{
        public:
                string Name;
                string Company;
                int Age;

                void IntroduceYourself(){
                        cout << "Name: " << Name << endl;
                        cout << "Company: " << Company << endl;
                        cout << "Age: " << Age << endl;
                }
};

int main(){
        Employee employee1;
        cout << "--- Employee 1 ---" << endl;
        employee1.Name = "Jivindra";
        employee1.Company = "Quest International University";
        employee1.Age = 29;
        employee1.IntroduceYourself();
        cout << endl;

        Employee employee2;
        cout << "--- Employee 2 ---" << endl;
        employee2.Name = "Ammar";
        employee2.Company = "AIMST University";
        employee2.Age = 39;
        employee2.IntroduceYourself();
        cout << endl;

        Employee employee3;
        cout << "--- Employee 3 ---" << endl;
        employee3.Name = "Len Pei Ying";
        employee3.Company = "QIU";
        employee3.Age = 22;
        employee3.IntroduceYourself();
        cout << endl;

        Employee employee4;
        cout << "--- Employee 4 ---" << endl;
        employee4.Name = "Chan Jade Qi";
        employee4.Company = "QIU";
        employee4.Age = 21;
        employee4.IntroduceYourself();
        cout << endl;

        Employee employee5;
        cout << "--- Employee 5 ---" << endl;
        employee5.Name = "Lim Jie Yee";
        employee5.Company = "QIU";
        employee5.Age = 21;
        employee5.IntroduceYourself();
        cout << endl;
}

