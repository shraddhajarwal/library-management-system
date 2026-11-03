#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
using namespace std;

// Simple custom linked list for books
struct Book {
    int id;
    string title, author;
    int total, available, borrowed;
};

struct Node {
    Book data;
    Node* next;
    Node(Book b) : data(b), next(NULL) {}
};

class BookList {
    Node* head;
public:
    BookList() : head(NULL) {}

    Book* find(int id) {
        for (Node* p = head; p; p = p->next)
            if (p->data.id == id) return &p->data;
        return NULL;
    }

    void add(Book b) {
        if (find(b.id)) { cout << "Book ID already exists.\n"; return; }
        Node* n = new Node(b);
        if (!head) head = n;
        else {
            Node* p = head;
            while (p->next) p = p->next;
            p->next = n;
        }
        cout << "Book added successfully.\n";
    }

    void show() {
        if (!head) { cout << "No books found.\n"; return; }
        cout << "\nID\tTitle\t\tAuthor\t\tAvailable/Total\n";
        for (Node* p = head; p; p = p->next)
            cout << p->data.id << "\t" << p->data.title << "\t\t"
                 << p->data.author << "\t\t" << p->data.available
                 << "/" << p->data.total << "\n";
    }

    void search(string word) {
        bool found = false;
        for (Node* p = head; p; p = p->next) {
            if (to_string(p->data.id) == word ||
                p->data.title.find(word) != string::npos ||
                p->data.author.find(word) != string::npos) {
                cout << "ID: " << p->data.id << " | " << p->data.title
                     << " | " << p->data.author << " | Available: "
                     << p->data.available << "\n";
                found = true;
            }
        }
        if (!found) cout << "Book not found.\n";
    }

    void edit(int id) {
        Book* b = find(id);
        if (!b) { cout << "Book not found.\n"; return; }
        cout << "New title: "; getline(cin, b->title);
        cout << "New author: "; getline(cin, b->author);
        cout << "New total copies: ";
        int n; cin >> n; cin.ignore();
        if (n < b->total - b->available) {
            cout << "Cannot set total below copies currently issued.\n";
            return;
        }
        int currentlyIssued = b->total - b->available;
        b->total = n;
        b->available = n - currentlyIssued;
        cout << "Book updated.\n";
    }

    void remove(int id) {
        Node *p = head, *prev = NULL;
        while (p && p->data.id != id) { prev = p; p = p->next; }
        if (!p) { cout << "Book not found.\n"; return; }
        if (p->data.available != p->data.total) {
            cout << "Cannot delete: some copies are issued.\n"; return;
        }
        if (prev) prev->next = p->next; else head = p->next;
        delete p;
        cout << "Book deleted.\n";
    }

    void save() {
        ofstream f("books.txt");
        for (Node* p = head; p; p = p->next) {
            Book b = p->data;
            f << b.id << "|" << b.title << "|" << b.author << "|"
              << b.total << "|" << b.available << "|" << b.borrowed << "\n";
        }
    }

    void load() {
        ifstream f("books.txt");
        string line;
        while (getline(f, line)) {
            stringstream s(line); string x;
            Book b;
            getline(s,x,'|'); b.id=stoi(x);
            getline(s,b.title,'|'); getline(s,b.author,'|');
            getline(s,x,'|'); b.total=stoi(x);
            getline(s,x,'|'); b.available=stoi(x);
            getline(s,x,'|'); b.borrowed=stoi(x);
            add(b);
        }
    }

    void reportMostBorrowed() {
        if (!head) { cout << "No books found.\n"; return; }
        Node* best = head;
        for (Node* p = head; p; p = p->next)
            if (p->data.borrowed > best->data.borrowed) best = p;
        cout << "Most borrowed: " << best->data.title
             << " (" << best->data.borrowed << " times)\n";
    }

    ~BookList() {
        while (head) { Node* t = head; head = head->next; delete t; }
    }
};

struct Member {
    int id;
    string name;
};

struct Transaction {
    int bookId, memberId;
    string issueDate, dueDate, returnDate;
    double fine;
    bool active;
};

vector<Member> members;
vector<Transaction> transactions;
const int BORROW_LIMIT = 3;
const int FINE_PER_DAY = 5;

int daysBetween(string a, string b) {
    tm x = {}, y = {};
    int ay,am,ad,by,bm,bd;
    if (sscanf(a.c_str(), "%d-%d-%d", &ay,&am,&ad) != 3 ||
        sscanf(b.c_str(), "%d-%d-%d", &by,&bm,&bd) != 3) return 0;
    x.tm_year=ay-1900; x.tm_mon=am-1; x.tm_mday=ad;
    y.tm_year=by-1900; y.tm_mon=bm-1; y.tm_mday=bd;
    time_t tx=mktime(&x), ty=mktime(&y);
    return (int)(difftime(ty,tx)/86400);
}

int countActive(int memberId) {
    int n=0;
    for (auto t:transactions) if (t.memberId==memberId && t.active) n++;
    return n;
}

Member* findMember(int id) {
    for (auto &m:members) if (m.id==id) return &m;
    return NULL;
}

void saveOther() {
    ofstream m("members.txt");
    for (auto x:members) m << x.id << "|" << x.name << "\n";
    ofstream t("transactions.txt");
    for (auto x:transactions)
        t << x.bookId << "|" << x.memberId << "|" << x.issueDate << "|"
          << x.dueDate << "|" << x.returnDate << "|" << x.fine << "|"
          << x.active << "\n";
}

void loadOther() {
    ifstream m("members.txt"); string line;
    while (getline(m,line)) {
        stringstream s(line); string x; Member a;
        getline(s,x,'|'); if(x.empty()) continue; a.id=stoi(x);
        getline(s,a.name,'|'); members.push_back(a);
    }
    ifstream t("transactions.txt");
    while (getline(t,line)) {
        stringstream s(line); string x; Transaction a;
        getline(s,x,'|'); if(x.empty()) continue; a.bookId=stoi(x);
        getline(s,x,'|'); a.memberId=stoi(x);
        getline(s,a.issueDate,'|'); getline(s,a.dueDate,'|');
        getline(s,a.returnDate,'|'); getline(s,x,'|'); a.fine=stod(x);
        getline(s,x,'|'); a.active=(x=="1"); transactions.push_back(a);
    }
}

void registerMember() {
    Member m;
    cout << "Member ID: "; cin >> m.id; cin.ignore();
    if (findMember(m.id)) { cout << "Member ID already exists.\n"; return; }
    cout << "Member name: "; getline(cin,m.name);
    members.push_back(m);
    cout << "Member registered.\n";
}

void manageMember(bool edit) {
    int id; cout << "Member ID: "; cin >> id; cin.ignore();
    for (int i=0; i<(int)members.size(); i++) {
        if (members[i].id == id) {
            if (edit) {
                cout << "New member name: "; getline(cin, members[i].name);
                cout << "Member updated.\n";
            } else {
                if (countActive(id)>0) {
                    cout << "Cannot delete: member has a book issued.\n"; return;
                }
                members.erase(members.begin()+i);
                cout << "Member deleted.\n";
            }
            return;
        }
    }
    cout << "Member not found.\n";
}

void issueBook(BookList &books) {
    int bid, mid; string issue,due;
    cout << "Book ID: "; cin >> bid;
    cout << "Member ID: "; cin >> mid; cin.ignore();
    Book* b=books.find(bid);
    if (!b) { cout << "Book not found.\n"; return; }
    if (!findMember(mid)) { cout << "Member not found.\n"; return; }
    if (b->available<=0) { cout << "Book is not available.\n"; return; }
    if (countActive(mid)>=BORROW_LIMIT) {
        cout << "Member has reached the limit of 3 books.\n"; return;
    }
    cout << "Issue date (YYYY-MM-DD): "; getline(cin,issue);
    cout << "Due date (YYYY-MM-DD): "; getline(cin,due);
    if (daysBetween(issue,due)<0) { cout << "Due date must be after issue date.\n"; return; }
    transactions.push_back({bid,mid,issue,due,"",0,true});
    b->available--; b->borrowed++;
    cout << "Book issued successfully.\n";
}

void returnBook(BookList &books) {
    int bid,mid; string date;
    cout << "Book ID: "; cin >> bid;
    cout << "Member ID: "; cin >> mid; cin.ignore();
    for (auto &t:transactions) {
        if (t.bookId==bid && t.memberId==mid && t.active) {
            cout << "Return date (YYYY-MM-DD): "; getline(cin,date);
            int late=daysBetween(t.dueDate,date);
            if (late<0) late=0;
            t.fine=late*FINE_PER_DAY;
            t.returnDate=date; t.active=false;
            Book* b=books.find(bid);
            if (b) b->available++;
            cout << "Book returned. Fine: Rs. " << t.fine << "\n";
            return;
        }
    }
    cout << "No active issue record found.\n";
}

void showHistory(int memberId) {
    bool found=false;
    cout << "\nBorrowing history:\n";
    for (auto t:transactions) if(t.memberId==memberId) {
        cout << "Book ID: " << t.bookId << " | Issued: " << t.issueDate
             << " | Due: " << t.dueDate << " | Returned: "
             << (t.returnDate.empty() ? "Not returned" : t.returnDate)
             << " | Fine: Rs. " << t.fine << "\n";
        found=true;
    }
    if(!found) cout << "No borrowing history.\n";
}

void overdueReport() {
    string today; cout << "Today's date (YYYY-MM-DD): "; getline(cin,today);
    bool found=false;
    for(auto t:transactions) if(t.active && daysBetween(t.dueDate,today)>0) {
        cout << "Book ID: " << t.bookId << " | Member ID: " << t.memberId
             << " | Days late: " << daysBetween(t.dueDate,today)
             << " | Current fine: Rs. "
             << daysBetween(t.dueDate,today)*FINE_PER_DAY << "\n";
        found=true;
    }
    if(!found) cout << "No overdue books.\n";
}

void memberActivityReport() {
    if(members.empty()) { cout << "No members registered.\n"; return; }
    for(auto m:members) {
        int total=0;
        for(auto t:transactions) if(t.memberId==m.id) total++;
        cout << m.id << " - " << m.name << " : " << total
             << " borrowing transaction(s)\n";
    }
}

void memberMenu(BookList &books) {
    int id; cout << "Enter your Member ID: "; cin >> id; cin.ignore();
    Member* m=findMember(id);
    if(!m) { cout << "Member not found. Ask librarian to register you.\n"; return; }
    int choice;
    do {
        cout << "\n--- MEMBER MENU (" << m->name << ") ---\n"
             << "1. View books\n2. Search book\n3. My borrowing history\n0. Back\nChoice: ";
        cin >> choice; cin.ignore();
        if(choice==1) books.show();
        else if(choice==2) { string q; cout << "Enter title, author or ID: "; getline(cin,q); books.search(q); }
        else if(choice==3) showHistory(id);
    } while(choice!=0);
}

void librarianMenu(BookList &books) {
    int c;
    do {
        cout << "\n--- LIBRARIAN MENU ---\n"
             << "1. Add book\n2. Edit book\n3. Delete book\n4. Search book\n"
             << "5. View all books\n6. Register member\n7. Issue book\n"
             << "8. Return book\n9. Edit member\n10. Delete member\n"
             << "11. Overdue report\n12. Most borrowed book\n"
             << "13. Member activity report\n14. Save records\n0. Back\nChoice: ";
        cin >> c; cin.ignore();
        if(c==1) {
            Book b;
            cout << "Book ID: "; cin >> b.id; cin.ignore();
            cout << "Title: "; getline(cin,b.title);
            cout << "Author: "; getline(cin,b.author);
            cout << "Total copies: "; cin >> b.total; cin.ignore();
            if(b.total<1) cout << "Copies must be at least 1.\n";
            else { b.available=b.total; b.borrowed=0; books.add(b); }
        } else if(c==2) { int id; cout<<"Book ID to edit: "; cin>>id; cin.ignore(); books.edit(id); }
        else if(c==3) { int id; cout<<"Book ID to delete: "; cin>>id; cin.ignore(); books.remove(id); }
        else if(c==4) { string q; cout<<"Title, author or ID: "; getline(cin,q); books.search(q); }
        else if(c==5) books.show();
        else if(c==6) registerMember();
        else if(c==7) issueBook(books);
        else if(c==8) returnBook(books);
        else if(c==9) manageMember(true);
        else if(c==10) manageMember(false);
        else if(c==11) overdueReport();
        else if(c==12) books.reportMostBorrowed();
        else if(c==13) memberActivityReport();
        else if(c==14) { books.save(); saveOther(); cout<<"All records saved.\n"; }
    } while(c!=0);
}

int main() {
    BookList books;
    books.load();
    loadOther();
    int choice;
    cout << "====================================\n"
         << "     LIBRARY MANAGEMENT SYSTEM\n"
         << "====================================\n";
    do {
        cout << "\n1. Librarian\n2. Member\n3. Save and Exit\nChoice: ";
        cin >> choice; cin.ignore();
        if(choice==1) librarianMenu(books);
        else if(choice==2) memberMenu(books);
        else if(choice==3) {
            books.save(); saveOther();
            cout << "Records saved. Thank you!\n";
        } else if(choice!=0) cout << "Invalid choice.\n";
    } while(choice!=3 && choice!=0);
    if (choice==0) {
        books.save(); saveOther();
        cout << "Records saved. Goodbye!\n";
    }
    return 0;
}
