#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>

#define FILENAME "file2.dat"
using namespace std;
const int M = 4; // Максимальная длина каталога бакетов

ifstream fi;
ofstream fo;

// 6911304 - Магическое число! Использовать для вставки.

struct Block;

int h(int val){
    return val%4;
}
// Структура студента
class Student { 
    int student_id = -1, group_id = -1;
    char Surname[20]{}, Name[20]{};
    char Patronymic[30]{};
public:
    Student() = default;
    Student(int stud_id,int gr_id, const char* Sur, const char* Nam, const char* Patr):
        student_id(stud_id), group_id(gr_id) {
        memcpy(Surname, Sur, sizeof Surname);
        memcpy(Name, Nam, sizeof Name);
        memcpy(Patronymic, Patr, sizeof Patronymic);
    };

    [[nodiscard]] int getId() const { return student_id; }
    static int getSizeOfSurname() { return sizeof Surname; }
    static int getSizeOfName() { return sizeof Name; }
    static int getSizeOfPatronymic() { return sizeof Patronymic; }
    void reset();

    void printInfo(){
        cout << "Id: " << student_id << endl << "Group id: " << group_id << endl << "FIO: " << Surname << ' ' << Name << ' ' << Patronymic << endl;
    }

    void setGroupId(int id){ group_id = id; }
    void setSurname(char* newSurname) { memcpy(Surname, newSurname, sizeof Surname); }
    void setName(char* newName) { memcpy(Name, newName, sizeof Name); }
    void setPatronymic(char* newPatronymic) { memcpy(Patronymic, newPatronymic, sizeof Patronymic); }
};

// Структура блока
struct Block{
    Student data[5]{};
    int length = 0;
    size_t next = 0;
    size_t cur = 0;

    Block() = default;
    explicit Block(size_t current){
        cur = current;
    }

    static Block getBlock(size_t filePointer);
    bool remove(int index);
    size_t append(Student student);
    static void saveBlock(Block block);
    void replaceStudent(int index, Student student) { data[index] = student; };
    Student getData(int i) { return data[i]; }
};

void Student::reset() {
    student_id = -1;
    group_id = -1;
    memset(Surname, ' ', sizeof Surname);
    memset(Name, ' ', sizeof Name);
    memset(Patronymic, ' ', sizeof Patronymic);
}

template< typename T >
std::string int_to_hex( T i )
{
    std::stringstream stream;
    stream << "0x"
           << std::setfill ('0') << std::setw(sizeof(T)*2)
           << std::hex << i;
    return stream.str();
}

Block Block::getBlock(size_t filePointer){
    fi.open(FILENAME, ios::binary | ios::in);
    fi.seekg(filePointer, ios::beg);
    Block b = Block();
    fi.read((char*)&b, sizeof(Block));
    if (fi.fail()){
        throw ios_base::failure (string("Error at getting a block at ") + int_to_hex<size_t>(filePointer));
    }
    if (fi.bad()){
        cout << "The integrity of the stream may have been affected\n";
    }
    fi.close();
    return b;
}

void Block::saveBlock(Block block) {
    fstream f(FILENAME, ios::out | ios::in | ios::binary);
    if (!f){
        cout << "Error opening file!" << endl;
        return;
    }
    f.seekp(block.cur, ios::beg);
    f.write((const char*)&block, sizeof(Block));
    if (f.fail()){
        throw ios_base::failure (string("Failed to save block at ") + to_string(block.cur) + string(" in file"));
    }
    f.close();
}

// Рекурсивно добавляет элемент в конец блока.
// Создаёт новые блоки при необходимости.
size_t Block::append(Student student){
    if (length < 5){
        data[length] = student;
        length++;
        try{
            saveBlock(*this);
        } catch (ios_base::failure f) {
            cout << f.what() << endl;
            return 0;
        }
        return cur;
    }
    if (this->next == 0) {
        this->next = this->cur + sizeof(Block);
        Block tmp = Block();
        Block::saveBlock(tmp);
    }
    try {
        size_t tmp = getBlock(next).append(student);
        return tmp;
    } catch (ios_base::failure f){
        cout << f.what() << endl;
        return 0;
    }
}

// Удаляет студента из блока.
// На вход требуется индекс в блоке.
// Если по какой либо причине не получилось удалить возвращает false, иначе true.
bool Block::remove(int index){
    if (index >= length || index < 0) return false;
    if (index == length-1){
        data[index].reset();
        length--;
        try{
            saveBlock(*this);
        } catch (ios_base::failure f) {
            cout << f.what() << endl;
            return false;
        }
        return true;
    }
    int i = index;
    while (i != length){
        if (i < length-1)
            data[i] = data[i+1];
        if (i == length-1)
            data[i].reset();
        i++;
    }
    length--;
    try{
        saveBlock(*this);
    } catch (ios_base::failure f) {
        cout << f.what() << endl;
        return false;
    }
    return true;
}

// Функция вставки студента в файл, требует структуру студента на вход.
void insert(Student student);
// Функция нахождения студента в файле;
// Возвращает блок в файле и запись студента,
// если ничего не найдено возвращает пустой блок и -1
std::pair<Block, int> find(int val);
// Функция удаления студента по номеру.
bool remove(int id);

void insert(Student student) {
    if(find(student.getId()).second != -1) {
        cout << "Student width this id already exists!" << endl;
        return;
    }
    Block g = Block::getBlock(0);
    g.append(student);
}

std::pair<Block, int> find(int val){
    Block g;
    try {
        g = Block::getBlock(0);
    } catch (ios_base::failure f){
        cout << f.what() << endl;
        return make_pair(Block(), -1);
    }
    int index = -1;
    while(index == -1){
        for(int i=0;i<=g.length;i++){
            if (g.getData(i).getId() == val){
                index = i;
                break;
            }
        }
        if (g.next == 0 || index != -1) break;
        try {
            g = Block::getBlock(g.next);
        } catch (ios_base::failure f){
            cout << f.what() << endl;
            return make_pair(Block(), -1);
        }
    }
    if (index == -1) return make_pair(Block(), -1);
    return std::make_pair(g, index);
}

bool remove(int id){
    auto a = find(id);
    Block needed = a.first;
    return needed.remove(a.second);
}

int main() {
    fstream f(FILENAME, ios::in | ios::out | ios::binary);
    if (!f){
        cout << "Error opening file" << endl;
        return 1;
    }
    f.close();
    int switchValue;
    bool exit = false;
    while (!exit){
        cout << "--------------------------------------------------" << endl;
        cout << "1) insert; 2) find; 3) remove; 4) update 9) quit" << endl;
        cout << "--------------------------------------------------" << endl;
        while (fscanf(stdin, " %d", &switchValue) != 1) {
            cout << "Wrong input!" << endl;
            cin.ignore(1000, '\n');
        }
        switch (switchValue) {
            case 1:{
                cin.ignore(1000, '\n');
                cout << "Type student id: ";
                int stud_id;
                while(fscanf(stdin, " %d", &stud_id) != 1){
                    cout << "Wrong input!" << endl;
                    cin.ignore(1000, '\n');
                }

                cin.ignore(1000, '\n');
                cout << "Type group id: ";
                int gr_id;
                while (fscanf(stdin, " %d", &gr_id) != 1)
                {
                    cout << "Wrong input!" << endl;
                    cin.ignore(1000, '\n');
                }
                char surname[20], name[20], patr[30];
                cout << "Enter surname name and patronymic with spaces: ";
                cin.ignore(1000, '\n');

                while(fscanf(stdin, " %20s %20s %30s", surname, name, patr) < 3)
                {
                    cout << "Wrong input!" << endl;
                    cin.ignore(1000, '\n');
                }
                Student stud(stud_id, gr_id, surname, name, patr);
                insert(stud);
                cout << "Done!" << endl;
                break;
            }
            case 2:{
                cout << "Type student id to find: ";
                cin.ignore(1000, '\n');
                while (fscanf(stdin, " %d", &switchValue) != 1)
                {
                    cout << "Wrong input!" << endl;
                    cin.ignore(1000, '\n');
                }
                auto values = find(switchValue);
                if (values.second == -1)
                    cout << "Not found!" << endl;
                else {
                    cout << "Block pointer " << values.first.cur << "; index " << values.second << endl;
                    values.first.getData(values.second).printInfo();
                }
                break;
            }
            case 3:{
                cout << "Type id of student to remove: ";
                cin.ignore(1000, '\n');
                while (fscanf(stdin, " %d", &switchValue) != 1)
                {
                    cout << "Wrong input!" << endl;
                    cin.ignore(1000, '\n');
                }
                if (remove(switchValue))
                    cout << "Success!" << endl;
                else
                    cout << "There is no student width this id!" << endl;

                break;
            }
            case 4:{
                cout << "Type id of student to update: ";
                while (fscanf(stdin, " %d", &switchValue) != 1)
                {
                    cout << "Wrong input!" << endl;
                    cin.ignore(1000, '\n');
                }
                auto a = find(switchValue);
                if(a.second == -1) {
                    cout << "Couldn't find this student!" << endl;
                    break;
                }
                bool ex = true;
                int innerSwitchValue;
                Block studentBlock = a.first;
                Student student = studentBlock.getData(a.second);

                while (ex){
                    cout << "---------------------------------------------" << endl;
                    student.printInfo();
                    cout << "---------------------------------------------" << endl;
                    cout << "1) group_id; 2) Surname; 3) Name; 4) Patronymic; 5) Exit" << endl;
                    cin >> innerSwitchValue;
                    switch (innerSwitchValue) {
                        case 1: {
                            cout << "Type new group id: ";
                            int new_id;
                            while (fscanf(stdin, " %d", &new_id) != 1)
                            {
                                cout << "Wrong input!" << endl;
                                cin.ignore(1000, '\n');
                            }
                            student.setGroupId(new_id);
                            break;
                        }
                        case 2: {
                            char newSurname[20];
                            char buf[32] = { 0 };
                            strcat(buf, " %");
                            char buf1[16] = { 0 };
                            itoa(Student::getSizeOfSurname(), buf1, 10);
                            strcat(buf, buf1);
                            strcat(buf, "s");
                            cout << "Type new surname: ";
                            while (fscanf(stdin, buf, newSurname) != 1)
                            {
                                cout << "Wrong input!" << endl;
                                cin.ignore(1000, '\n');
                            }
                            cout << newSurname << endl;
                            student.setSurname(newSurname);
                            break;
                        }
                        case 3: {
                            cout << "Type new name: ";
                            char newName[20];
                            char buf[32] = { 0 };
                            strcat(buf, " %");
                            char buf1[16] = { 0 };
                            itoa(Student::getSizeOfName(), buf1, 10);
                            strcat(buf, buf1);
                            strcat(buf, "s");
                            while (fscanf(stdin, buf, newName) != 1)
                            {
                                cout << "Wrong input!" << endl;
                                cin.ignore(1000, '\n');
                            }
                            cout << newName << endl;
                            student.setName(newName);
                            break;
                        }
                        case 4: {
                            cout << "Type new patronymic: ";
                            char newPatronymic[30];
                            char buf[32] = { 0 };
                            strcat(buf, " %");
                            char buf1[16] = { 0 };
                            itoa(Student::getSizeOfPatronymic(), buf1, 10);
                            strcat(buf, buf1);
                            strcat(buf, "s");
                            while (fscanf(stdin, buf, newPatronymic) != 1)
                            {
                                cout << "Wrong input!" << endl;
                                cin.ignore(1000, '\n');
                            }
                            cout << newPatronymic << endl;
                            student.setPatronymic(newPatronymic);
                            break;
                        }
                        case 5: {
                            ex = false;
                            break;
                        }
                        default: {
                            cout << "Wrong input!" << endl;
                        }
                    }
                }
                studentBlock.replaceStudent(a.second, student);
                Block::saveBlock(studentBlock);
                break;
            }
            case 9:{
                exit = !exit;
                break;
            }

            default: {
                cout << "Wrong input!" << endl;
            }
        }
    }
    return 0;
}