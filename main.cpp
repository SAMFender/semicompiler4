//group work Saman A.Mirhosseni and UPC
//semi compiler ---- c programming 94 final project
#include <iostream>
#include <fstream>//for reading from file and writing on file
#include <string>//for string usage instant of char *
#include <vector>
using namespace std;
int row_index = 0;
bool exist;
int brace_do_not_owner[3000]={0};//for {} which does not has a owner-->set all to 0
int num_index = 0;
ofstream ir_generated_file;
enum family_group{keyword,single_punc,double_punc,var_type,op_comp,op_arith,op_logic,num_i,num_f,chars,var_name};//group the words in codes:
//keyword {main , if , else , while* , void , null, true , false , return} =0
//single_punc{, , ;} =1
//double_punc{() , {} }=2
//var_type{int , float ,bool } =3
//op_comp {> , < , == ,!= } =4
//op_arith {=,+,-,/,*} =5
//op_logic {&& ,||} =6
//num_i {1,2,3,0,681,52,....} =7
//num_f {0.89 ,9.12,....} =8
//chars{'c','a',....} =9
//var_name {mehi ,....}=10 --> error words also save here
bool is_integer(string check)//check if the string is an integer or not
{
    if(check[0]<49 || check[0]>57)//check the first index if it is 0 or not
    {
        return false;
    }
    for(int i=1;i<check.size();i++)//check the other indexes by ASCII code
    {
        if(check[i]<48 || check[i]>57)
        {
            return false;
        }
    }
    return true;
}
bool is_float(string check)//check if the string is a float or not
{
    int dot_position=check.find(".");//fine the dot position
    if(dot_position==string::npos)//check if it has dot or not 12.987606
    {
        return false;
    }
    else if(dot_position==0)//check if its dot is in the first or not
    {
        return false;
    }
    string part1=check.substr(0,dot_position);//separate the digits before dot
    string part2=check.substr(dot_position+1);//separate the digits after dot
    for(int i=0;i<part1.size();i++)//check
    {
        if(part1[i]<48 || part1[i]>57)//check the other indexes by ASCII code
        {
            return false;
        }
    }
    for(int i=0;i<part2.size();i++)
    {
        if(part2[i]<48 || part2[i]>57)//check the other indexes by ASCII code
        {
            return false;
        }
    }
    return true;
}
int to_intiger(string use)
{
    int change=0;
    int temp_num=0;
    for(int i=0;i<use.size();i++)
    {
        temp_num=use[i];
        change=(change*10)+(temp_num-48);//0 ASCII code is 48
    }
    return change;
}
float to_float(string use)
{
    float change_first=0;
    int dot_position=use.find(".");
    int temp_num=0;
    for(int i=0;i<dot_position;i++)
    {
        temp_num=use[i];
        change_first=(change_first*10)+(temp_num-48);//0 ASCII code is 48
    }
    for(int i=dot_position+1;i<use.size();i++)//instant of using continue
    {
        temp_num=use[i];
        change_first=(change_first*10)+(temp_num-48);//0 ASCII code is 48
    }
    int ten=1;
    for(int i=1;i<use.size()-dot_position;i++)//find out how many digits are after the dot
    {
        ten*=10;
    }
    float change=change_first/ten;
    return change;
}
bool is_valid(string check)
{
    if(!( (check[0]>64 && check[0]<91) || (check[0]>96 && check[0]<123) ))//check if the first index is alpha
    {
        return false;
    }
    for(int i=1;i<check.size();i++)//check if other indexes is alpha or number
    {
        if(!( (check[i]>47 && check[i]<58) || (check[i]>64 && check[i]<91) || (check[i]>96 && check[i]<123) ))
        {
            return false;
        }
    }
    return true;
}

struct tokens//the link list which save all of words except the white words(space,tab,enter)
{
    int word_index;//save the word number use in syntax checking
    family_group family;//save the group as in enum
    string atr;//save the exact word --> caution : we do not save the (') for char type
    int line_number;//save the line number use in error handling and IR Code
    bool has_error;//if the word has an a error it must be true
    bool has_checked;//when the word check we set it to true
    tokens *next;//save the next struct address
} *tok_head=new tokens;//the head of link list
struct variable_name//the link list for symbol table
{
    int index;//save the index of struct
    bool unkhown;//if a variable do not have a value it must be true
    string name;//save the variable name as atr string in tokens
    string type;//int , char , bool ,float
    string value;//12 true c 89.023
    variable_name *next;//save the next struct address
} *var_head=new variable_name;//the head of link list
variable_name * search_sym_table(string what){
    variable_name * temp = var_head;
    while(temp != NULL){
        if(temp -> name == what){
            return temp;
        }
    }
    return NULL;
}

void add_last_func(string name1,string type1,string value1,bool unknown1)
{
    variable_name * temp = var_head;
    while(temp -> next != NULL)
    {
        temp = temp ->next;
    }
    temp -> next = new variable_name;
        temp -> name = name1;
        temp -> type = type1;
        temp -> value = value1;
        temp -> unkhown = unknown1;
        temp -> index = row_index;
        row_index++;
    temp -> next -> next = NULL;
}


void set_brace_array()
{
    tokens *tok_temp=tok_head;
    int brace_temp[3000]={0};
    int brace_index[3000]={0};
    int counter=0;
    while(tok_temp->next!=NULL)
    {
        if(tok_temp->family==double_punc)
        {
            if(tok_temp->atr=="{")
            {
                brace_temp[counter]=1;
                brace_index[counter]=tok_temp->word_index;
                counter++;
            }
            else if(tok_temp->atr=="}")
            {
                brace_temp[counter]=-1;
                brace_index[counter]=tok_temp->word_index;
                counter++;
            }
        }
        tok_temp=tok_temp->next;
    }
    delete tok_temp;
    while(true)
    {
        int open=-1;
        int close=-1;
        int i;
        for(i=counter-1;brace_temp[i]!=1 || i>-1;i--)
        {
        }
        if(brace_temp[i]==1)
        {
            open=brace_temp[i];
        }
        else
        {
            //
            break;
        }
        for(i=0;brace_temp[i]!=-1 || i<counter;i++)
        {
        }
        if(brace_temp[i]==-1)
        {
            close=brace_temp[i];
        }
        else
        {
            //
            break;
        }
        brace_temp[open]=2;
        brace_temp[close]=2;
    }
}

int UPC_make_it_error_and_checked(int which_index)//make all tokens between two ; error and check--->caution : can not use for main ,if , else , while
{
    int start=0,finish=0;
    tokens *tok_temp=tok_head;
    while(tok_temp->word_index!=which_index)
    {
        if(tok_temp->atr==";")
        {
            start=tok_temp->word_index;
        }
        tok_temp=tok_temp->next;
    }
    while(tok_temp->next!=NULL)
    {
        if(tok_temp->atr==";")
        {
            finish=tok_temp->word_index;
            break;
        }
        tok_temp=tok_temp->next;
    }
    start++;
    tok_temp=tok_head;
    while(tok_temp->next!=NULL)
    {
        if(tok_temp->word_index>=start && tok_temp->word_index<=finish)
        {
            tok_temp->has_checked=true;
            tok_temp->has_error=true;
        }
        tok_temp=tok_temp->next;
    }
    return (finish+1);
}
void UPC_pre_process()
{
    //-----the variable we need for check and find main function and include file
    int void_position;
    int main_position;
    int sharp_position;//#
    int include_position;
    int file_name_position;
    int include_1_position;//<
    int include_2_position;//>
    bool void_existance = false;
    bool main_existance = false;
    bool sharp_existance = false;
    bool include_existance = false;
    bool include_1_existance = false;
    bool include_2_existance = false;
    string user_main;
    string user_include;
    string include_file_name;
    //------read main.c and save it in string
    ifstream read_main ("main.c",ios::in);
    if(!read_main)
    {
        cerr<<"ERROR in file opening check your main.c"<<endl;
        return;
    }
    while(true)
    {
        string temp;
        if(read_main.eof())
        {
            break;
        }
        getline(read_main,temp);
        user_main = user_main + temp;
        user_main += "\n";
    }
    cout<<"reading main.c ...."<<endl;
    read_main.close();
    //----find what we need and error handling
    main_position = user_main.find("main");
    if(main_position!=string::npos)
    {
        main_existance=true;
        void_position = user_main.find("void");
        if(void_position!=string::npos)
        {
            void_existance=true;
        }
        else
        {
            cerr<<"error : You do not define the main type"<<endl;
        }
    }
    else
    {
        cerr<<"error : Your program does not contain main"<<endl;
    }
    sharp_position = user_main.find("#");
    if(sharp_position!=string::npos)
    {
       sharp_existance=true;
       include_position = user_main.find("include");
       if(include_position!=string::npos)
       {
            include_existance=true;
            include_1_position= user_main.find("<");
            if(include_1_position!=string::npos)
            {
                include_1_existance=true;
                include_2_position= user_main.find(">");
                if(include_2_position!=string::npos)
                {
                    include_2_existance=true;
                    if(include_position-sharp_position!=2 || include_1_position-include_position!=8 || (include_2_position-include_1_position>=20&& include_2_position-include_1_position<2))
                    {
                        cerr<<"error : un correct order for include !"<<endl;
                        return;
                    }
                }
                else
                {
                    cerr<<"error : You do not use the open include (>) for header name"<<endl;
                    return;
                }
            }
            else
            {
                cerr<<"error : You do not use the open include (<) for header name"<<endl;
                return;
            }
        }
        else
        {
            cerr<<"error : You dont use the include after your pre process (#) "<<endl;
            return;
        }
    }
    else
    {
        include_position = user_main.find("include");
        if(include_position!=string::npos)
        {
            cerr<<"error : Your program do not contain #  but has an include !"<<endl;
            return;
        }
    }
    //------writing together
    if(sharp_existance)
    {
        if(include_existance)
        {
            include_file_name=user_main.substr(include_1_position+2,include_2_position-include_1_position-3);
            char temp_name[ include_file_name.size()];
            for(int i=0;i< include_file_name.size();i++)
            {
                temp_name[i]= include_file_name[i];
            }
            ifstream read_include(temp_name,ios::in);
            if(!read_include)
            {
                cerr<<"ERROR in file opening check your including file "<<endl;
                return;
            }
            while(true)
            {
                string temp;
                if(read_include.eof())
                {
                    break;
                }
                getline(read_include,temp);
                user_include = user_include + temp;
                user_include += "\n";
            }
            cout<<"reading include file ...."<<endl;
            user_include.erase(user_include.size()-2,2);
            user_main.erase(sharp_position,include_2_position-sharp_position+1);
            user_main.erase(user_main.size()-2,2);
            read_include.close();
            //-------------------
            ofstream go_compile("compile.c",ios::out);
            if(!go_compile)
            {
                cerr<<"ERROR in create a new file for compile "<<endl;
            }
            go_compile<<user_include<<user_main;
            cout<<"writing codes in new file ...."<<endl;
            go_compile.close();
        }
    }
}
void UPC_full_tokens()
{
    ifstream compile("compile.c",ios::in);
    if(!compile )
    {
        cerr<<"ERROR : can not open the compile file !"<<endl;
        return;
    }
    string line;
    string word;
    int line_counter=0;
    int word_counter=0;
    tokens *tok_temp=tok_head;
    while(true)
    {
        getline(compile,line);
        if(!line.empty())
        {
            ofstream file_temp_write("temp.SM&UPC",ios::out);
            if(!file_temp_write )
            {
                cerr<<"ERROR : can not open the temp file !"<<endl;
                return;
            }
            file_temp_write<<line;
            line_counter++;
            file_temp_write.close();
            ifstream file_temp_read("temp.SM&UPC",ios::in);
            if(!file_temp_read )
            {
                cerr<<"ERROR : can not open the temp file !"<<endl;
                return;
            }
            while(true)
            {
                file_temp_read>>word;
                word_counter++;
                //---------checking keyword
                if(word=="main")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="if")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="else")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="void")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="null")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="true")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="false")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="return")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }

                else if(word=="while")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=keyword;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking single_punc
                else if(word==",")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=single_punc;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word==";")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=single_punc;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking double_punc
                else if(word=="(")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=double_punc;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word==")")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=double_punc;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="{")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=double_punc;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="}")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=double_punc;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking var_type
                else if(word=="int")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=var_type;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="bool")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=var_type;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="char")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=var_type;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="float")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=var_type;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking op_comp
                else if(word==">")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_comp;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="<")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_comp;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="==")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_comp;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="!=")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_comp;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking op_arith
                else if(word=="=")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_arith;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="+")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_arith;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="-")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_arith;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="*")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_arith;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="/")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_arith;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking op_logic
                else if(word=="&&")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_logic;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                else if(word=="||")
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=op_logic;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking chars
                else if(word.find("'")!=string::npos)
                {
                    int quote_first=word.find("'");
                    if(quote_first==0)
                    {
                        int quote_second=word.find("'",quote_first+1);
                        if(quote_second!=string::npos)
                        {
                            if(quote_second-quote_first==2)
                            {
                                tok_temp->next=NULL;
                                tok_temp->atr=word.substr(1,1);
                                tok_temp->family=chars;
                                tok_temp->line_number=line_counter;
                                tok_temp->has_error=false;
                                tok_temp->has_checked=false;
                                tok_temp->word_index=word_counter;
                            }
                            else
                            {
                                cerr<<"an un known word in line :: "<<line_counter<<endl;
                                tok_temp->next=NULL;
                                tok_temp->atr=word;
                                tok_temp->family=var_name;
                                tok_temp->line_number=line_counter;
                                tok_temp->has_error=true;
                                tok_temp->has_checked=false;
                                tok_temp->word_index=word_counter;
                            }
                        }
                        else
                        {
                            cerr<<"an un known word in line :: "<<line_counter<<endl;
                            tok_temp->next=NULL;
                            tok_temp->atr=word;
                            tok_temp->family=var_name;
                            tok_temp->line_number=line_counter;
                            tok_temp->has_error=true;
                            tok_temp->has_checked=false;
                            tok_temp->word_index=word_counter;
                        }
                    }
                    else
                    {
                        cerr<<"an un known word in line :: "<<line_counter<<endl;
                        tok_temp->next=NULL;
                        tok_temp->atr=word;
                        tok_temp->family=var_name;
                        tok_temp->line_number=line_counter;
                        tok_temp->has_error=true;
                        tok_temp->has_checked=false;
                        tok_temp->word_index=word_counter;
                    }
                }
                //---------checking num_i
                else if(is_integer(word))
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=num_i;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking num_f
                else if(is_float(word))
                {
                    tok_temp->next=NULL;
                    tok_temp->atr=word;
                    tok_temp->family=num_f;
                    tok_temp->line_number=line_counter;
                    tok_temp->has_error=false;
                    tok_temp->has_checked=false;
                    tok_temp->word_index=word_counter;
                }
                //---------checking var_name
                else
                {
                    if(is_valid(word))
                    {
                        tok_temp->next=NULL;
                        tok_temp->atr=word;
                        tok_temp->family=var_name;
                        tok_temp->line_number=line_counter;
                        tok_temp->has_error=false;
                        tok_temp->has_checked=false;
                        tok_temp->word_index=word_counter;
                    }
                    else
                    {
                        cerr<<"an un known word in line :: "<<line_counter<<endl;
                        tok_temp->next=NULL;
                        tok_temp->atr=word;
                        tok_temp->family=var_name;
                        tok_temp->line_number=line_counter;
                        tok_temp->has_error=true;
                        tok_temp->has_checked=false;
                        tok_temp->word_index=word_counter;
                    }
                }
                tok_temp->next=new tokens;
                tok_temp=tok_temp->next;
                //***********************************************
                if(file_temp_read.eof())
                {
                    cout<<"reading line : "<<line_counter<<endl;
                    break;
                }
            }
        }
        if(compile.eof())
        {
            cout<<"The file tokening done successfully :-)"<<endl;
            break;
        }
    }
    tok_temp=NULL;
    tok_temp=tok_head;
    int next_check=1;
    while(tok_temp->next!=NULL)
    {
        if(tok_temp->has_error==true && tok_temp->word_index>=next_check)
        {
            next_check=UPC_make_it_error_and_checked(tok_temp->word_index);
        }
        tok_temp=tok_temp->next;
    }
    compile.close();
}
void UPC_show()
{
    tokens *t=tok_head;
    while(t->next!=NULL)
    {
        cout<<"the atr is : "<<t->atr<<endl;
        cout<<"the family is : "<<t->family<<endl;
        cout<<"the line number is : "<<t->line_number<<endl;
        cout<<"the word number is : "<<t->word_index<<endl;
        cout<<"the error is : "<<t->has_error<<endl;
        cout<<"the checked is : "<<t->has_checked<<endl;
        cout<<"----------------------------------------"<<endl;
        t=t->next;
    }
}

void SAM_full_variable_name()
{
    tokens * tok_temp = tok_head;
    variable_name * var_temp = var_head;
    variable_name * help_row;
    tokens * help;
    tokens * type_define;
    int sym_index = 0;
    while (tok_temp != NULL)
    {
        if (tok_temp -> has_error == false){
        if(tok_temp -> family == var_type)
            {
                help = tok_temp;
                type_define = tok_temp;
                tok_temp -> has_checked == true;
                tok_temp = tok_temp -> next;
            if(tok_temp -> family == var_name)
            {
                help = tok_temp;
                tok_temp = tok_temp -> next;
                if(tok_temp -> family == single_punc)
                {
                    if(tok_temp -> atr == ";")
                    {
                        help_row = search_sym_table(help -> atr);
                        if(help_row == NULL)
                        {

                        }
                        else
                        {
                            cerr << "line number : "<< help -> line_number << help -> atr << " Previously declared here" << endl;
                            UPC_make_it_error_and_checked(help -> word_index);
                            help -> has_checked = true;
                            tok_temp -> has_checked = true;
                            tok_temp -> has_error = true;
                            help -> has_error = true;
                            type_define -> has_error = true;
                            tok_temp = tok_temp -> next;
                        }
                    }
                    else if(tok_temp -> atr == ",")
                    {

                    }
                }
                else if(tok_temp -> atr == "=")
                {

                }
                else
                {

                }
            }
            else{
                cerr << "line number : " <<help -> line_number <<" expected identifier after " << help -> atr << " variable type token " << endl;
                UPC_make_it_error_and_checked(tok_temp -> word_index);
                tok_temp -> has_checked = true;
                tok_temp = tok_temp -> next;
            }
        }
        else
        {
          tok_temp = tok_temp -> next;
        }
        }
        else
        {
            tok_temp -> has_checked == true;
            tok_temp = tok_temp -> next;
        }
    }
}
void memory_handel(tokens * what ,bool arith){
    ir_generated_file.open("IR",ios::app);
    if(what -> family == num_f || what -> family == num_i)
    {
        if(arith = false){
        cout << "T_" << num_index << " := " << what -> atr << endl;
        }
        else
        {
        cout << " T_" << num_index;
        }
        num_index++;
    }
    else if(what -> family == var_name){
        cout << " T" << " ";
    }
}

void ir_generation(tokens * start_point){
    ir_generated_file.open("IR");
    tokens * alternate;
    tokens * help;
    tokens * op_or_not;
    int num_index = 0;
    int t_counter = 0;
    variable_name * row;
    tokens * temp = start_point;
    if(temp -> family == var_type){
        temp = temp -> next;
        alternate = temp -> next;
        if(alternate -> atr == "="){
            bool first_num = false;
            bool second_num = false;
            alternate = alternate -> next;
            op_or_not = alternate -> next;
            if(op_or_not -> family == op_arith){
               if(alternate -> family == num_f || alternate -> family == num_i)
                {
                    memory_handel(alternate,false);
                    help = alternate;
                    alternate = op_or_not -> next;
                    if(alternate -> family == num_f || alternate -> family == num_i){
                    memory_handel (alternate,false);
                    }
                    memory_handel(temp,false);
                    cout << " := ";
                    memory_handel(help,true);
                    cout << " " << op_or_not -> atr <<" ";
                    memory_handel(alternate,true);
                    cout << endl;
                }
               else if(alternate -> family == var_name){
                    help = alternate;
                    alternate = op_or_not -> next;
                    if(alternate -> family == num_f || alternate -> family == num_i){
                            memory_handel(alternate,false);
                    }
                    memory_handel(temp,false);
                    cout << ":= ";
                    memory_handel (help,false);
                    cout << op_or_not -> atr;
                    memory_handel(alternate,true);
                    cout << endl;
               }
            }
        }
        alternate = alternate -> next;
        temp = alternate -> next;
        }
        else if(temp -> family = var_name)
        {
            alternate = temp -> next;
            help = alternate -> next;
            op_or_not = help -> next;
            tokens * for_fun = op_or_not -> next;
            if(help -> family == num_i || help -> family == num_f){
                memory_handel(help,false);
            }
            if(for_fun -> family == num_i || for_fun -> family == num_f){
                memory_handel(for_fun,false);
            }
            memory_handel(temp,true);
            ir_generated_file << " = ";
            memory_handel(help,true);
            ir_generated_file <<" "<< op_or_not<<" ";
            memory_handel(for_fun,true);
            ir_generated_file << endl;
            for_fun = for_fun -> next;
            temp = for_fun -> next;
        }

}
int main()
{
    var_head = NULL;
    UPC_pre_process();
    UPC_full_tokens();
    UPC_show();
    SAM_full_variable_name();
    return 0;
}
