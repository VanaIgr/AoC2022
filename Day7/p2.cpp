#include<iostream>
#include<fstream>
#include<cstring>

int parseNumber(std::istream &s) {
    auto n = 0;
    decltype(s.peek()) c;
    while(
        (c = s.peek()) != std::char_traits<char>::eof()
        & c >= '0' & c <= '9'
    ) n = n*10 + (s.get() - '0');
    return n;
}

char const *parseString(std::istream &s) {
    auto const startPos = s.tellg();
    int count = 0;
    decltype(s.peek()) c;
    while(
        (c = s.peek()) != std::char_traits<char>::eof()
        & (c >= 'a' & c <= 'z')
    ) {
        count++;
        s.ignore(1);
    }

    s.seekg(startPos, s.beg);

    auto const string = new char[count + 1];
    string[count] = '\0';

    for(int i = 0; i < count; i++) {
        string[i] = s.get();
    }

    return string;
}

struct Subfolder;

struct Folder {
    char const *name;
    int filesSize;
    Subfolder *subfolders;
    Folder *parent;
};

struct Subfolder {
    Folder cur;
    Subfolder *next;
};


static Folder &includeFolder(char const *const name, Folder &parent) {
    auto subfolder = &parent.subfolders;
    while(*subfolder != nullptr) {
        auto &folder = (**subfolder).cur;
        if(strcmp(name, folder.name) == 0) {
            delete[] name;
            return folder;
        }
        else {
            auto const nextSubfolder = &(**subfolder).next;
            subfolder = nextSubfolder;
        }
    }
    *subfolder = new Subfolder{ { name, 0, nullptr, &parent }, nullptr };
    return (**subfolder).cur;
}

static Folder *findImFolder(char const*const name, Folder &start) {
    //if(strcmp(name, start.name) == 0) return &start;
    auto subfolder = start.subfolders;
    while(subfolder != nullptr) {
        auto &folder = subfolder->cur;
        if(strcmp(name, folder.name) == 0) return &folder;
        //auto const folderCand = findFolder(name, folder);
        //if(folderCand != nullptr) return folderCand;
        subfolder = subfolder->next;
    }
    return nullptr;
}

static void printFolder(std::ostream &s, Folder const &o, int const indent) {
    using namespace std;
    s.width(indent);
    s << ' ';
    s.width(1);
    s << "dir " << o.name << '\n';
    s.width(indent);
    s << ' ';
    s.width(1);
    s << "im files size: " << o.filesSize << '\n';
    auto subfolder = o.subfolders;
    while(subfolder != nullptr) {
        printFolder(s, subfolder->cur, indent + 2);
        subfolder = subfolder->next;
    }
}

static std::ostream &operator<<(std::ostream &s, Folder const &o) {
    printFolder(s, o, 0);
    return s;
}

struct CalcResult {
    decltype(Folder::filesSize) folderSize;
};

static int updateFolderSize(Folder &folder) {
    auto folderSize = decltype(CalcResult::folderSize){};
    auto subfolders = folder.subfolders;
    while(subfolders) {
        auto const result = updateFolderSize(subfolders->cur);
        folderSize += result;
        subfolders = subfolders->next;
    }
    folderSize += folder.filesSize;
    folder.filesSize = folderSize;
    return folderSize;
}

static int findSmallestFolderSize(Folder &folder, int minSize) {
    auto smallestFolderSize = folder.filesSize;
    auto subfolders = folder.subfolders;
    while(subfolders) {
        auto const folderSize = findSmallestFolderSize(
            subfolders->cur, minSize
        );
        if(
            smallestFolderSize > folderSize
            && folderSize > minSize
        ) {
            smallestFolderSize = folderSize;
        }
        subfolders = subfolders->next;
    }
    return smallestFolderSize;
}


static int calcFoldersAnswer(Folder &folder) {
    auto const totalSize = updateFolderSize(folder);
    auto const needToFree = 30'000'000 - (70'000'000 - totalSize);
    return findSmallestFolderSize(folder, needToFree);
}

int main() {
    auto stream = std::ifstream{"p1.txt"};

    Folder root{ "/" };
    Folder *curFolder = nullptr;

    int i = 0;
    while(stream.peek() != std::char_traits<char>::eof()) {
        auto c = stream.peek();
        
        /*std::cout << "command " << i << ":\n";
        std::cout << root;
        if(curFolder != 0) std::cout << "curDirName: " << curFolder->name << '\n';
        std::cout << "------------------------------------------------------------\n";
        i++;*/

        if(c != '$') return 99; 
        stream.ignore(2);

        c = stream.peek();
        if(c == 'c') {
            stream.ignore(3);
            c = stream.peek();
            if(c == '/') {
                curFolder = &root;
                stream.ignore(2);
            }
            else if(c == '.') {
                stream.ignore(3);
                curFolder = curFolder->parent;
            }
            else {
                auto const name = parseString(stream);
                stream.ignore(1); //newline

                auto const newCurFolder = findImFolder(name, *curFolder);
                if(newCurFolder == nullptr) {
                    std::cout << *curFolder;
                    auto const pos = stream.tellg();
                    stream.seekg(0, stream.beg);
                    int lines = 0;
                    for(auto i = 0; i < pos; ++i) {
                        if(stream.get() == '\n') lines++;
                    }
                    std::cout << "lines: " << lines << '\n';
                    return 1;
                }
                curFolder = newCurFolder;
            }
        }
        else if(c == 'l') {
            curFolder->filesSize = 0;
            stream.ignore(3);
            while(true) {
                c = stream.peek();
                if(c == '$') break;
                else if(c == 'd') {
                    stream.ignore(4);
                    auto const dirName = parseString(stream);
                    includeFolder(dirName, *curFolder);
                    stream.ignore(1);
                }
                else if(c >= '0' & c <= '9') {
                    auto const fileSize = parseNumber(stream);
                    stream.ignore(1);
                    while((c = stream.peek()) != std::char_traits<char>::eof() && c != '\n') stream.ignore(1);
                    stream.ignore(1);
                    curFolder->filesSize += fileSize;
                }
                else if(c == -1) break;
                else return 101;
            }
        }
        else return 100;
    }

    std::cout << calcFoldersAnswer(root);

    //std::cout << root;

    return 0;
}
