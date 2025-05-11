#include <iostream>
#include <fstream>
#include <format>
using namespace std;

class OS{
    private:
        char M[999904][4];          //Physical Memory
        char IR[4];                 //Instruction Register (4 bytes
        // char R1[4];              //General Purpose Register 1 (4 bytes)
        // char R2[4];              //General Purpose Register 2 (4 bytes)
        static const int R1 = 999900;
        static const int R2 = 999901;
        static const int CS = 999902;
        static const int DS = 999903;
        int IC;                     //Instruction Counter Register (2 bytes)
        int SI;                     //Interrupt
        bool C;                     //Toggle (1 byte)
        bool CF;                    //Carry Flag
        char buffer[41];

        void GD();
        void PD();
        void H();
        void LR();
        void SR();
        void CR();
        void BT();

        // additional instruction set
        void AD();
        void SB();
        void IM();
        void DM();
        void ML();
        void DV();
        void BC();

        void AN();
        void OR();
        void NT();
        void RS();
        void LS();

        int fetchAddress(int segmentR = DS, bool enableSegment = true);
        
    public:
        void init();
        void LOAD();
        void MOS();
        void Execute();
        
        fstream infile;
        fstream outfile;

        // For Debugging
        void MEMDump(string filename, int segment = 0, int count = 1);
    };

//Memory Dump
void OS::MEMDump(string filename, int segment, int count)
{
    cout << "[+] Memory Dump..." << endl;
    fstream dumpfile;
    dumpfile.open(filename, ios::out);
    dumpfile << ".M" << endl;
    for (int k = 0; k < count; k++)
    {
        for(int i = 0; i < 100; i++)
        {
            dumpfile << format("{:04}", segment + k) << "-" << format("{:02}", i) << ": ";
            for(int j = 0; j < 4; j++)
            {
                dumpfile << M[(segment + k) * 100 + i][j];
            }
            dumpfile << endl;
        }
        dumpfile << endl;
    }
    dumpfile << ".REG" << endl;
    dumpfile << "IR: " << IR[0] << IR[1] << IR[2] << IR[3] << endl;
    dumpfile << "IC: " << format("{:02}", IC) << endl;
    dumpfile << "C : " << C << endl;
    dumpfile << "CF: " << CF << endl;
    for (int i = 0; i < 4; i++)
    {
        dumpfile << "R" << i+1 << ": ";
        for (int j = 0; j < 4; j++)
        {
            dumpfile << M[999900+i][j];
        }
        dumpfile << endl;
    }
    dumpfile.close();
}


//initiallise everything to ' '
void OS::init()
{
    cout<<endl<< "[+] Initialise Program..." <<endl;

    for(int i = 0; i < 999904; i++)
    {    
        for(int j = 0; j < 4; j++ )
        {  
            M[i][j] = ' ';
        }
        
    }
   
    for (int i = 0; i < 4; i++)
    {
        IR[i]    = '*';
        M[CS][i] = '0';
        M[DS][i] = '0';
    };
    C  = false;
    CF = false;
}

int OS::fetchAddress(int segmentR, bool enableSegment)
{
    int address;
    if      (IR[2] == 'R' && IR[3] == '1') address = R1; // rax
    else if (IR[2] == 'R' && IR[3] == '2') address = R2;
    else if (IR[2] == 'C' && IR[3] == 'S') address = CS;
    else if (IR[2] == 'D' && IR[3] == 'S') address = DS;
    else if (IR[2] == 'I' && IR[3] == '1') address = (M[R1][2] - '0') * 10 + (M[R1][3] - '0'); // [rax]
    else if (IR[2] == 'I' && IR[3] == '2') address = (M[R2][2] - '0') * 10 + (M[R2][3] - '0');
    else address = (IR[2] - '0') * 10 + (IR[3] - '0');

    if (!enableSegment) return address;
    if (address >= 999900) return address; // if address is an register

    int base = (M[segmentR][0] - '0') * 1000 + (M[segmentR][1] - '0') * 100 + (M[segmentR][2] - '0') * 10 + (M[segmentR][3] - '0');
    address += base * 100;
    
    return address;
}

// MOS
void OS::MOS()
{
    cout << "[+] System Interrupt..."<<endl;
    if(SI == 1)
    {
        GD();
    }
    else if(SI == 2)
    {
        PD();
    }
    else if(SI == 3)
    {
        H();
    }
    SI = 0;
}

void OS::GD()
{
    int address = fetchAddress();
    for(int i=0; i < 40; i++) buffer[i] = ' ';
    infile.getline(buffer, 40);
    for(int i=0; i < 40; i++)
    {
        if (buffer[i] == '\n' || buffer[i] == '\0') buffer[i] = ' ';
    }

    int k = 0;
    for(int i = 0; i < 10; i++)
    {
        for(int j = 0; j<4; ++j)
        {
            M[address+i][j] = buffer[k];
            k++;
        }
    }
}

void OS::PD()
{
    int address = fetchAddress();

    for(int i = address; i < address+10; i++)
    {
        for(int j = 0; j<4; j++)
        {
            outfile << M[i][j];
        }
    }
    outfile << endl;
}

void OS::H()
{
    cout << "[+] Halting Program..." << endl;
    outfile << endl;
}

void OS::LR()
{
    int address = fetchAddress();
    for(int i = 0; i<4; i++)
    {
        M[R1][i] = M[address][i];
    }
}

void OS::SR()
{
    int address = fetchAddress();
    for(int i = 0; i<4; i++)
    {
        M[address][i] = M[R1][i];
    }
}

void OS::CR()
{
    int address = fetchAddress();
    for(int i = 0; i<4; i++)
    {
        if(M[R1][i] != M[address][i])
        {
            C = false;
            break;
        }
        else
        {
            C = true;
        }
    }
}

void OS::BT()
{
    int address = fetchAddress(CS);
    if(C)
    {
        IC = address;
    }
}



void OS::AD()
{
    int address = fetchAddress();
    bool carry = false;
    for (int i = 3; i >= 0; i--)
    {
        int ans = (M[R1][i] - '0') + (M[address][i] - '0');
        if (carry) ans += 1;

        carry = false;
        if (ans > 9)
        {
            carry = true;
            ans -= 10;
        }

        M[R1][i] = (char) (ans + (int) '0');
    }

    CF = carry;
}

void OS::SB()
{
    int address = fetchAddress();
    bool borrow = false;
    for (int i = 3; i >= 0; i--)
    {
        int ans = (M[R1][i] - '0') - (M[address][i] - '0');
        if (borrow) ans -= 1;

        borrow = false;
        if (ans < 0)
        {
            borrow = true;
            ans += 10;
        }

        M[R1][i] = (char) (ans + (int) '0');
    }

    CF = borrow;
}

void OS::IM()
{
    int address = fetchAddress();
    bool carry = true;
    for (int i = 3; i >= 0; i--)
    {
        int ans = (M[address][i] - '0');
        if (carry) ans += 1;

        carry = false;
        if (ans > 9)
        {
            carry = true;
            ans -= 10;
        }

        M[address][i] = (char) (ans + (int) '0');
    }

    CF = carry;
}

void OS::DM()
{
    int address = fetchAddress();
    bool borrow = true;
    for (int i = 3; i >= 0; i--)
    {
        int ans = (M[address][i] - '0');
        if (borrow) ans -= 1;

        borrow = false;
        if (ans < 0)
        {
            borrow = true;
            ans += 10;
        }

        M[address][i] = (char) (ans + (int) '0');
    }

    CF = borrow;
}

void OS::ML()
{
    int address = fetchAddress();
    int m1 = 0;
    int m2 = 0;
    for (int i = 0; i < 4; i++)
    {
        m1 = m1 * 10 + (M[R1][i] - '0');
        m2 = m2 * 10 + (M[address][i] - '0');
    }
    int ans = m1 * m2;
    for (int i = 3; i >= 0; i--)
    {
        M[R1][i] = (char) (ans % 10 + (int) '0');
        ans /= 10;
    }
    for (int i = 3; i >= 0; i--)
    {
        M[R2][i] = (char) (ans % 10 + (int) '0');
        ans /= 10;
    }
}

void OS::DV()
{
    int address = fetchAddress();
    int m1 = 0;
    int m2 = 0;
    for (int i = 0; i < 4; i++)
    {
        m1 = m1 * 10 + (M[R1][i] - '0');
        m2 = m2 * 10 + (M[address][i] - '0');
    }

    if (m2 == 0)
    {
        cout << "[-] Division by Zero Error..." << endl;
        H();
        return;
    }

    int ans = m1 / m2;
    int rem = m1 % m2;
    for (int i = 3; i >= 0; i--)
    {
        M[R1][i] = (char) (ans % 10 + (int) '0');
        ans /= 10;
    }
    for (int i = 3; i >= 0; i--)
    {
        M[R2][i] = (char) (rem % 10 + (int) '0');
        rem /= 10;
    }
}

void OS::BC()
{
    if (CF)
    {
        int address = fetchAddress(CS);
        IC = address;
    }
}


void OS::AN()
{
    int address = fetchAddress();
    for (int i = 0; i < 4; i++)
    {
        M[R1][i] =  M[R1][i] & M[address][i];
    }
}

void OS::OR()
{
    int address = fetchAddress();
    for (int i = 0; i < 4; i++)
    {
        M[R1][i] =  M[R1][i] | M[address][i];
    }
}

void OS::NT()
{
    int address = fetchAddress();
    for (int i = 0; i < 4; i++)
    {
        M[address][i] =  ~M[address][i];
    }
}

void OS::RS()
{
    int count = fetchAddress(DS, false); // only (00-99) or I1 or I2 is allowed (no R1 or R2)
    for (int i = 3; i >= count; i--)
    {
        M[R1][i] =  M[R1][i-count];
    }
    for (int i = 0; i < count; i++)
    {
        M[R1][i] = '\0';
    }
}

void OS::LS()
{
    int count = fetchAddress(DS, false); // only (00-99) or I1 or I2 is allowed (no R1 or R2)
    for (int i = 0; i < 4-count; i++)
    {
        M[R1][i] =  M[R1][i+count];
    }
    for (int i = 4-count; i < 4; i++)
    {
        M[R1][i] = '\0';
    }
}


// Execution
void OS::Execute()
{
    cout<<endl<<"[+] Executing Program..."<<endl;
    MEMDump("backups/memory_dump_before.txt", 0, 2);

    while (1)
    {
        for (int i = 0; i<4; i++)
        {
            IR[i] = M[IC][i];
        }
        IC++;

        if (IR[0] == 'G' && IR[1] == 'D')
        {
            SI = 1;
            MOS();
        }
        else if(IR[0] == 'P' && IR[1] == 'D')
        {
            SI = 2;
            MOS();
        }
        else if(IR[0] == 'H' && IR[1] == ' ')
        {
            SI = 3;
            MOS();
            break;
        }
        else if(IR[0] == 'L' && IR[1] == 'R')
        {
            LR();
        }
        else if(IR[0] == 'S' && IR[1] == 'R')
        {
            SR();
        }
        else if(IR[0] == 'C' && IR[1] == 'R')
        {
            CR();
        }
        else if(IR[0] == 'B' && IR[1] == 'T')
        {
            BT();
        }
        else if(IR[0] == 'A' && IR[1] == 'D')
        {
            AD();
        }
        else if(IR[0] == 'S' && IR[1] == 'B')
        {
            SB();
        }
        else if(IR[0] == 'I' && IR[1] == 'M')
        {
            IM();
        }
        else if(IR[0] == 'D' && IR[1] == 'M')
        {
            DM();
        }
        else if(IR[0] == 'M' && IR[1] == 'L')
        {
            ML();
        }
        else if(IR[0] == 'D' && IR[1] == 'V')
        {
            DV();
        }
        else if(IR[0] == 'A' && IR[1] == 'N')
        {
            AN();
        }
        else if(IR[0] == 'O' && IR[1] == 'R')
        {
            OR();
        }
        else if(IR[0] == 'N' && IR[1] == 'T')
        {
            NT();
        }
        else if(IR[0] == 'R' && IR[1] == 'S')
        {
            RS();
        }
        else if(IR[0] == 'L' && IR[1] == 'S')
        {
            LS();
        }
        else if(IR[0] == 'B' && IR[1] == 'C')
        {
            BC();
        }
        else
        {
            cout << "[-] Invalid Instruction..."<<endl;
            H();
            break;
        }
    }

    MEMDump("backups/memory_dump_after.txt", 0, 2);
}

// Load Function
void OS::LOAD()
{
    cout<<"[+] Reading Data..."<<endl;
    int x=0;
    do
    {
        //clear buffer
        for(int i=0; i < 41; i++) buffer[i]=' ';

        infile.getline(buffer, 41);
        for(int i=0; i < 40; i++)
        {
            if (buffer[i] == '\n' || buffer[i] == '\0') buffer[i] = ' ';
        }

        for(int k=0;k<41;k++)
           cout<<buffer[k];
        
        if(buffer[0] == '$' && buffer[1] == 'A' && buffer[2] == 'M' && buffer[3] == 'J')
        { 
            init();
            
        }
        else if(buffer[0] == '$' && buffer[1] == 'D' && buffer[2] == 'T' && buffer[3] == 'A')
        {
            IC = 00;
            Execute();
        }
        else if(buffer[0] == '$' && buffer[1] == 'E' && buffer[2] == 'N' && buffer[3] == 'D')
        {
            cout<<endl<< "[+] Exiting Program..." << endl;
            x=0;
        }
        else
        {
            cout<<endl<<"[+] Loading Program card..."<<endl;
            int k = 0;
            
            for(; x <999900; ++x)
            {
                for(int j = 0 ; j < 4; ++j)
                {
                    M[x][j] = buffer[k];
                    k++;
                }
                
                if (k == 40 || buffer[k] == ' ' || buffer[k] == '\n')
                {
                    x++;
                    break;
                }
                
            }   
             
        }

    } while(!infile.eof());         //continues to take input till eof
        
    
}

int main()
{ 
    OS os;

    os.infile.open("input.txt", ios::in);
    os.outfile.open("output.txt", ios::out);

    if(!os.infile)
    {
        cout<<"Failure"<<endl;
    }
      
    os.LOAD();

    return 0;
}