#include <iostream>
#include <fstream>
#include <vector>
#include <format>
#include <string>
#include <random>
#include <set>
using namespace std;

#define MEMS 99995
#define REGS 4

class OS {
    private:
        char M[MEMS][4];           //Physical Memory
        char IR[4];                 //Instruction Register (4 bytes)
        char PCB[4];                //Process Control Block (4 bytes)

        // char R1[4];              //General Purpose Register 1 (4 bytes)
        // char R2[4];              //General Purpose Register 2 (4 bytes)
        static const int R1 = 99991;
        static const int R2 = 99992;
        static const int CS = 99993;
        static const int DS = 99994;
        int IC;                     //Instruction Counter
        bool C;                     //Toggle (1 byte)
        bool CF;                    //Carry Flag
        char buffer[41];

        int SI;                     //Service Interrupt
        int TI;                     //Timer Interrupt
        int PI;                     //Protection Interrupt

        int state = 0;

        mt19937 rgen;
        uniform_int_distribution<int> rrange;

        set<int> allocatedAddresses; 
        int lastAllocated = 0; // Debug

        bool GD();
        bool PD();
        bool H();
        bool LR();
        bool SR();
        bool CR();
        bool BT();
        bool JT();

        // additional instruction set
        bool AD();
        bool SB();
        bool IM();
        bool DM();
        bool ML();
        bool DV();
        bool BC();
        bool JC();

        bool AN();
        bool OR();
        bool NT();
        bool RS();
        bool LS();

        int allocateMemoryFrame();
        int resolveValidPageFault(int segmentR = DS, bool skipIR = false, int psudoAddress = 0);
        int fetchAddress(int segmentR = DS, bool enableSegment = true);
        void freeMemoryFrame(int address);
        
    public:
        void init(int pid, int ttl, int tll);
        void LOAD();
        bool MOS();
        void Execute();
        
        fstream infile;
        fstream outfile;

        // For Debugging
        void MEMDump(string filename);

        OS() : rgen(random_device{}()), rrange(0, (int)((MEMS - REGS) / 10)) {
            for (int i = 0; i < MEMS; i++) memset(M[i], ' ', 4);
        }
};

void writeIntToWord(int n, char* word)
{
    for (int i = 3; i >= 0; i--)
    {
        word[i] = (char)(n % 10 + '0');
        n /= 10;
    }
}

int readIntFromWord(char* word)
{
    int ans = 0;
    for (int i = 0; i < 4; i++)
    {
        ans = ans * 10 + (word[i] - '0');
    }
    return ans;
}

int readIntFrom2Byte(char* word)
{
    int ans = 0;
    for (int i = 0; i < 2; i++)
    {
        ans = ans * 10 + (word[i] - '0');
    }
    return ans;
}

void fillBlockWithSpaces(char (*block)[4])
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            block[i][j] = ' ';
        }
    }
}

//Memory Dump
void OS::MEMDump(string filename)
{
    cout << "[+] Memory Dump..." << endl;
    fstream dumpfile;
    dumpfile.open(filename, ios::out);
    dumpfile << ".M" << endl;
    for(int i = 0; i < MEMS - REGS; i++)
    {
        dumpfile << format("{:04}", i) << ": ";
        for(int j = 0; j < 4; j++)
        {
            dumpfile << M[i][j];
        }
        dumpfile << endl;
    }
    dumpfile << ".REG" << endl;
    dumpfile << "PCB: " << PCB[0] << PCB[1] << PCB[2] << PCB[3] << endl;
    dumpfile << "IR: " << IR[0] << IR[1] << IR[2] << IR[3] << endl;
    dumpfile << "IC: " << format("{:02}", IC) << endl;
    dumpfile << "C : " << C << endl;
    dumpfile << "CF: " << CF << endl;
    for (int i = 0; i < REGS; i++)
    {
        dumpfile << "R" << i+1 << ": ";
        for (int j = 0; j < 4; j++)
        {
            dumpfile << M[MEMS-REGS+i][j];
        }
        dumpfile << endl;
    }
    dumpfile.close();
}


//initiallise everything to ' '
void OS::init(int pid, int ttl, int tll)
{
    cout<<endl<< "[+] Initialise Program..." <<endl;

    for (int i = 0; i < 4; i++)
    {
        IR[i]    = '*';
        M[CS][i]   = '0';
        M[DS][i]   = '0';
        M[R1][i]   = ' ';
        M[R2][i]   = ' ';
    };
    M[DS][3] = '1';

    C  = false;
    CF = false;
    IC = 0;

    SI = 0;
    TI = 0;
    PI = 0;

    // PCB Structure
    // 0: PID (Process Identifier)
    // 1: TTL (Total Time Left)
    // 2: TLL (Total Line Limit)
    // 3: Process Mode/Status (0 = Ready, 1 = Wait, 2 = exited)
    // 4: Input File line number
    // 5: Output File line number
    // 6: Number of PTRs process using
    // 7: PTRs entries list start address
    // 8: saved CPU state address
    // 9: <not-used>

    // CPU State
    // 0: IC
    // 1: R1
    // 2: R2
    // 3: CS
    // 4: DS
    // 5: C
    // 6: CF
    // 7: <not-used>
    // 8: <not-used>
    // 9: <not-used>

    int PCBaddress = allocateMemoryFrame();
    fillBlockWithSpaces(&M[PCBaddress * 10]);

    int PTRSaddress = allocateMemoryFrame();
    fillBlockWithSpaces(&M[PTRSaddress * 10]);

    int PTRaddress = allocateMemoryFrame();
    fillBlockWithSpaces(&M[PTRaddress * 10]);
    writeIntToWord(PTRaddress, M[PTRSaddress * 10 + 0]);

    int CPUstateaddress = allocateMemoryFrame();
    fillBlockWithSpaces(&M[CPUstateaddress * 10]);

    writeIntToWord(pid, M[PCBaddress * 10 + 0]);  // PID
    writeIntToWord(ttl, M[PCBaddress * 10 + 1]);  // TTL
    writeIntToWord(tll, M[PCBaddress * 10 + 2]);  // TLL
    writeIntToWord(0,   M[PCBaddress * 10 + 3]);  // Status
    //
    //
    writeIntToWord(1,   M[PCBaddress * 10 + 6]);  // PTRS length
    writeIntToWord(PTRSaddress, M[PCBaddress * 10 + 7]); // PTRSaddress
    writeIntToWord(CPUstateaddress, M[PCBaddress * 10 + 8]); // CPU State address

    writeIntToWord(PCBaddress, PCB);

    MEMDump("backups/memdump_" + std::to_string(pid) + ".txt");
}

int OS::allocateMemoryFrame()
{
    // for (int i = 0 ; i < (MEMS - REGS) / 10; i++) // Debug
    // {
    //     if (!allocatedAddresses.contains(i))
    //     {
    //         allocatedAddresses.insert(i);
    //         fillBlockWithSpaces(&M[i * 10]);
    //         return i;
    //     }
    // }

    // int frame = rrange(rgen);
    int frame = lastAllocated; // Debug
    while (allocatedAddresses.contains(frame))
    {
        // frame = rrange(rgen);
        frame = ++lastAllocated;
    }
    allocatedAddresses.insert(frame);
    fillBlockWithSpaces(&M[frame * 10]);
    return frame;
}

int OS::resolveValidPageFault(int segmentR, bool skipIR, int psudoAddress)
{
    int address;
    if (skipIR) address = psudoAddress;
    else if (IR[2] == 'R' && IR[3] == '1') address = R1; // rax
    else if (IR[2] == 'R' && IR[3] == '2') address = R2;
    else if (IR[2] == 'C' && IR[3] == 'S') address = CS;
    else if (IR[2] == 'D' && IR[3] == 'S') address = DS;
    else if (IR[2] == 'I' && IR[3] == '1') address = (M[R1][2] - '0') * 10 + (M[R1][3] - '0'); // [rax]
    else if (IR[2] == 'I' && IR[3] == '2') address = (M[R2][2] - '0') * 10 + (M[R2][3] - '0');
    else address = (IR[2] - '0') * 10 + (IR[3] - '0');

    if (address >= MEMS - REGS) return address; // if address is an register

    int PTRSindex = readIntFromWord(M[segmentR]);
    int PCBaddress = readIntFromWord(PCB);
    int PTRSlength = readIntFromWord(M[PCBaddress * 10 + 6]);

    // cout << "[*] PTRSindex: " << PTRSindex << endl; // for debugging

    if (PTRSindex > PTRSlength)
    {
        cout << "[-] Page Fault (Invalid)" << endl;
        return -1;
    }
    else if (PTRSindex == PTRSlength)
    {
        int PTRSaddress = readIntFromWord(M[PCBaddress * 10 + 7]);
        int nPTRSaddress;
        while (PTRSindex > 8)
        {
            nPTRSaddress = readIntFromWord(M[PTRSaddress * 10 + 9]);
            if (nPTRSaddress < 0 || nPTRSaddress * 10 > MEMS - REGS)
            {
                int newFrame = allocateMemoryFrame();
                writeIntToWord(newFrame, M[PTRSaddress * 10 + 9]);
                fillBlockWithSpaces(&M[newFrame * 10]);
            }
            PTRSaddress = readIntFromWord(M[PTRSaddress * 10 + 9]);
            PTRSindex -= 9;
        }

        int PTRaddress = readIntFromWord(M[PTRSaddress * 10 + PTRSindex]);
        if (PTRaddress < 0 || PTRaddress * 10 > MEMS - REGS)
        {
            int newFrame = allocateMemoryFrame();
            writeIntToWord(newFrame, M[PTRSaddress * 10 + PTRSindex]);
            fillBlockWithSpaces(&M[newFrame * 10]);

            writeIntToWord(PTRSlength + 1, M[PCBaddress * 10 + 6]);
        }
    }

    int PTRSaddress = readIntFromWord(M[PCBaddress * 10 + 7]);
    while (PTRSindex > 8)
    {
        PTRSaddress = readIntFromWord(M[PTRSaddress * 10 + 9]);
        PTRSindex -= 9;
    }

    int PTRaddress = readIntFromWord(M[PTRSaddress * 10 + PTRSindex]);

    // cout << "[*] Meta: " << segmentR << " : " << PCBaddress << " : " << PTRSlength << " : " << PTRSaddress << " : " << PTRSindex << " : " << M[segmentR][0]<<M[segmentR][1]<<M[segmentR][2]<<M[segmentR][3]  << endl; // for debugging

    int FrameAddress = readIntFromWord(M[PTRaddress * 10 + (int)(address / 10)]);
    if (FrameAddress < 0 || FrameAddress * 10 > MEMS - REGS)
    {
        int newFrame = allocateMemoryFrame();
        writeIntToWord(newFrame, M[PTRaddress * 10 + (int)(address / 10)]);
        fillBlockWithSpaces(&M[newFrame * 10]);
    }
    FrameAddress = readIntFromWord(M[PTRaddress * 10 + (int)(address / 10)]);

    return FrameAddress * 10 + (address % 10);
}

int OS::fetchAddress(int segmentR, bool enableSegment)
{
    // cout << "[+] Fetching Address: "<< IR[2] << IR[3] << " : " << segmentR << " : " << readIntFromWord(M[segmentR]) << endl; // for debugging
    int address;
    if      (IR[2] == 'R' && IR[3] == '1') address = R1; // rax
    else if (IR[2] == 'R' && IR[3] == '2') address = R2;
    else if (IR[2] == 'C' && IR[3] == 'S') address = CS;
    else if (IR[2] == 'D' && IR[3] == 'S') address = DS;
    else if (IR[2] == 'I' && IR[3] == '1') address = (M[R1][2] - '0') * 10 + (M[R1][3] - '0'); // [rax]
    else if (IR[2] == 'I' && IR[3] == '2') address = (M[R2][2] - '0') * 10 + (M[R2][3] - '0');
    else
    {
        if (IR[2] < '0' || IR[2] > '9' || IR[3] < '0' || IR[3] > '9')
        {
            cout << "[-] Invalid Address" << endl;
            PI = 2;
            return MOS();
        }
        address = (IR[2] - '0') * 10 + (IR[3] - '0');
    }

    if (!enableSegment) return address;
    if (address >= MEMS - REGS) return address; // if address is an register

    int PTRSindex = readIntFromWord(M[segmentR]);
    int PCBaddress = readIntFromWord(PCB);
    int PTRSlength = readIntFromWord(M[PCBaddress * 10 + 6]);

    // cout << "[*] Meta: " << PCBaddress << " : " << PTRSlength << " : " << PTRSindex << " : " << M[segmentR][0]<<M[segmentR][1]<<M[segmentR][2]<<M[segmentR][3]  << endl; // for debugging

    if (PTRSindex >= PTRSlength)
    {
        PI = 3;
        if (MOS())
        {
            return fetchAddress(segmentR, enableSegment);
        }
        else
        {
            return -1; // Page Fault (Invalid)
        }
    }

    int PTRSaddress = readIntFromWord(M[PCBaddress * 10 + 7]);
    while (PTRSindex > 8)
    {
        PTRSaddress = readIntFromWord(M[PTRSaddress * 10 + 9]);
        PTRSindex -= 9;
    }

    int PTRaddress = readIntFromWord(M[PTRSaddress * 10 + PTRSindex]);

    int FrameAddress = readIntFromWord(M[PTRaddress * 10 + (int)(address / 10)]);
    if (FrameAddress < 0 || FrameAddress > MEMS - REGS)
    {
        PI = 3;
        if (MOS())
        {
            return fetchAddress(segmentR, enableSegment);
        }
        else
        {
            return -1; // Page Fault (Invalid)
        }
    }
    
    return FrameAddress * 10 + (address % 10);
}

void OS::freeMemoryFrame(int frame)
{
    auto it = allocatedAddresses.find(frame);
    if (it == allocatedAddresses.end()) return;

    allocatedAddresses.erase(it);
}

// MOS
bool OS::MOS()
{
    cout << "[+] System Interrupt" << " SI:" << SI << " PI:" << PI << " TI:" << TI << endl;
    if (PI == 1)
    {
        cout << "➤ [-] Operation Error" << endl;
        H(); return false;
    }
    else if (PI == 2)
    {
        cout << "➤ [-] Operand Error" << endl;
        H(); return false;
    }
    else if (PI == 3 && SI == 1)
    {
        cout << "➤ [-] Page Fault (Valid)" << endl;
        if (resolveValidPageFault() < 0)
        {
            cout << "➤ [-] Page Fault (Invalid)" << endl;
            H(); return false;
        }
    }
    else if (PI == 3)
    {
        cout << "➤ [-] Page Fault (Invalid)" << endl;
        H(); return false;
    }
    else if (TI == 0 && SI == 1)
    {
        GD();
    }
    else if (TI == 0 && SI == 2)
    {
        PD();
    }
    else if (TI == 0 && SI == 3)
    {
        H(); return false;
    }
    else if (TI == 2 && SI == 0)
    {
        cout << "➤ [-] Time Limit Exceeded" << endl;
        H(); return false;
    }
    else if (TI == 2 && SI == 1)
    {
        cout << "➤ [-] Out Of Data Cards" << endl;
        H(); return false;
    }
    else if (TI == 2 && SI == 2)
    {
        PD();
        cout << "➤ [-] Line Limit Exceeded" << endl;
        H(); return false;
    }
    else if (TI == 2 && SI == 3)
    {
        cout << "➤ [-] Time Limit Exceeded" << endl;
        H(); return false;
    }


    SI = 0;
    TI = 0;
    PI = 0;
    return true;
}

bool OS::GD()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

    for(int i=0; i < 40; i++) buffer[i] = ' ';
    infile.getline(buffer, 40);
    for(int i=0; i < 40; i++)
    {
        if (buffer[i] == '\n' || buffer[i] == '\0') buffer[i] = ' ';
    }

    if (buffer[0] == '$' && buffer[1] == 'E' && buffer[2] == 'N' && buffer[3] == 'D')
    {
        TI = 2;
        return MOS();
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
    return true;
}

bool OS::PD()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

    for(int i = address; i < address+10; i++)
    {
        for(int j = 0; j<4; j++)
        {
            outfile << M[i][j];
        }
    }
    outfile << endl;
    return true;
}

bool OS::H()
{
    cout << "[+] Halting Program..." << endl;
    outfile << endl;

    // Free program's memory
    int PCBaddress = readIntFromWord(PCB);

    int PTRSlength = readIntFromWord(M[PCBaddress * 10 + 6]);
    int PTRSaddress = readIntFromWord(M[PCBaddress * 10 + 7]);

    for (int i = 0; i < PTRSlength; i++)
    {
        int PTRaddress = readIntFromWord(M[PTRSaddress * 10 + (i % 9)]);

        for (int j = 0; j < 10; j++)
        {
            int frame = readIntFromWord(M[PTRaddress * 10 + j]);
            if (frame < 0 || frame * 10 > MEMS - REGS)
            {
                break;
            }

            freeMemoryFrame(frame);
        }

        freeMemoryFrame(PTRaddress);

        if (i % 9 == 8)
        {
            int nPTRSaddress = readIntFromWord(M[PTRSaddress * 10 + 9]);
            if (nPTRSaddress < 0 || nPTRSaddress * 10 > MEMS - REGS)
            {
                break;
            }

            freeMemoryFrame(PTRSaddress);
            PTRSaddress = nPTRSaddress;

        }
    }

    freeMemoryFrame(PTRSaddress);
    freeMemoryFrame(readIntFromWord(M[PCBaddress * 10 + 8]));
    freeMemoryFrame(PCBaddress);

    cout << "[+] Freeed Associated memory" << endl;

    return false;
}

bool OS::LR()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)
    for(int i = 0; i<4; i++)
    {
        M[R1][i] = M[address][i];
    }
    return true;
}

bool OS::SR()
{
    SI = 1;
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)
    SI = 0;

    for(int i = 0; i<4; i++)
    {
        M[address][i] = M[R1][i];
    }
    return true;
}

bool OS::CR()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

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
    return true;
}

bool OS::BT()
{
    int address = fetchAddress(CS, false);
    if (address == -1) return false; // Page Fault (Invalid)

    if(C)
    {
        IC = address;
    }
    return true;
}

bool OS::JT()
{
    int address = fetchAddress(R2, false);
    if (address == -1) return false; // Page Fault (Invalid)

    if(C)
    {
        IC = address;
    }
    return true;
}


bool OS::AD()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

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
    return true;
}

bool OS::SB()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

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
    return true;
}

bool OS::IM()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

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
    return true;
}

bool OS::DM()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

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
    return true;
}

bool OS::ML()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

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
    return true;
}

bool OS::DV()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

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
        PI = 2;
        return MOS();
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
    return true;
}

bool OS::BC()
{
    if (CF)
    {
        int address = fetchAddress(CS);
        if (address == -1) return false; // Page Fault (Invalid)

        IC = address;
    }
    return true;
}

bool OS::JC()
{
    if (CF)
    {
        int address = fetchAddress(R1);
        if (address == -1) return false; // Page Fault (Invalid)

        IC = address;
    }
    return true;
}


bool OS::AN()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

    for (int i = 0; i < 4; i++)
    {
        M[R1][i] =  M[R1][i] & M[address][i];
    }
    return true;
}

bool OS::OR()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

    for (int i = 0; i < 4; i++)
    {
        M[R1][i] =  M[R1][i] | M[address][i];
    }
    return true;
}

bool OS::NT()
{
    int address = fetchAddress();
    if (address == -1) return false; // Page Fault (Invalid)

    for (int i = 0; i < 4; i++)
    {
        M[address][i] =  ~M[address][i];
    }
    return true;
}

bool OS::RS()
{
    // cout << "[*] R1: " << M[R1][0] << M[R1][1] << M[R1][2] << M[R1][3] << endl; // for debugging
    int count = fetchAddress(DS, false); // only (00-99) or I1 or I2 is allowed (no R1 or R2)
    // cout << "[*] RS-Count: " << count << endl; // for debugging
    if (count == -1) return false; // Page Fault (Invalid)

    for (int i = 3; i >= count; i--)
    {
        M[R1][i] =  M[R1][i-count];
    }
    for (int i = 0; i < count; i++)
    {
        M[R1][i] = '\0';
    }
    return true;
}

bool OS::LS()
{
    int count = fetchAddress(DS, false); // only (00-99) or I1 or I2 is allowed (no R1 or R2)
    // cout << "[*] LS-Count: " << count << endl; // for debugging
    if (count == -1) return false; // Page Fault (Invalid)

    for (int i = 0; i < 4-count; i++)
    {
        M[R1][i] =  M[R1][i+count];
    }
    for (int i = 4-count; i < 4; i++)
    {
        M[R1][i] = '\0';
    }
    return true;
}


// Execution
void OS::Execute()
{
    cout<<endl<<"[+] Executing Program..."<<endl;
    MEMDump("backups/memory_dump_before.txt");

    while (1)
    {
        int PCBaddress = readIntFromWord(PCB);
        int ttc = readIntFromWord(M[PCBaddress * 10 + 1]);
        if (ttc == 0)
        {
            TI = 2;
            if (!MOS()) break;
        }
        writeIntToWord(ttc - 1, M[PCBaddress * 10 + 1]);

        int address = resolveValidPageFault(CS, true, IC);

        for (int i = 0; i<4; i++)
        {
            IR[i] = M[address][i];
        }
        // cout << IC << " " << IR[0] << IR[1] << IR[2] << IR[3] << "  " << address << endl; // for debugging
        IC++;
        IC = IC % 100;

        if (IR[0] == 'G' && IR[1] == 'D')
        {
            SI = 1;
            if (!MOS()) break;
        }
        else if(IR[0] == 'P' && IR[1] == 'D')
        {
            SI = 2;
            if (!MOS()) break;
        }
        else if(IR[0] == 'H' && IR[1] == ' ')
        {
            SI = 3;
            if (!MOS()) break;
        }
        else if(IR[0] == 'L' && IR[1] == 'R')
        {
            if (!LR()) break;
        }
        else if(IR[0] == 'S' && IR[1] == 'R')
        {
            if (!SR()) break;
        }
        else if(IR[0] == 'C' && IR[1] == 'R')
        {
            if (!CR()) break;
        }
        else if(IR[0] == 'B' && IR[1] == 'T')
        {
            if (!BT()) break;
        }
        else if(IR[0] == 'A' && IR[1] == 'D')
        {
            if (!AD()) break;
        }
        else if(IR[0] == 'S' && IR[1] == 'B')
        {
            if (!SB()) break;
        }
        else if(IR[0] == 'I' && IR[1] == 'M')
        {
            if (!IM()) break;
        }
        else if(IR[0] == 'D' && IR[1] == 'M')
        {
            if (!DM()) break;
        }
        else if(IR[0] == 'M' && IR[1] == 'L')
        {
            if (!ML()) break;
        }
        else if(IR[0] == 'D' && IR[1] == 'V')
        {
            if (!DV()) break;
        }
        else if(IR[0] == 'A' && IR[1] == 'N')
        {
            if (!AN()) break;
        }
        else if(IR[0] == 'O' && IR[1] == 'R')
        {
            if (!OR()) break;
        }
        else if(IR[0] == 'N' && IR[1] == 'T')
        {
            if (!NT()) break;
        }
        else if(IR[0] == 'R' && IR[1] == 'S')
        {
            if (!RS()) break;
        }
        else if(IR[0] == 'L' && IR[1] == 'S')
        {
            if (!LS()) break;
        }
        else if(IR[0] == 'B' && IR[1] == 'C')
        {
            if (!BC()) break;
        }
        else
        {
            // cout << IR[0] << IR[1] << " : Invalid Instruction" << endl;
            PI = 1;
            if (!MOS()) break;
        }

        // cout << "======================================" << endl;
    }

    MEMDump("backups/memory_dump_after.txt");
}

// Load Function
void OS::LOAD()
{
    cout<<"[+] Reading Data..."<<endl;
    int x=0;
    int prvS=0;
    do
    {
        //clear buffer
        for (int i=0; i < 41; i++) buffer[i]=' ';

        infile.getline(buffer, 41);
        for (int i=0; i < 40; i++)
        {
            if (buffer[i] == '\n' || buffer[i] == '\0') buffer[i] = ' ';
        }

        for (int k=0;k<41;k++)
           cout<<buffer[k];
        
        if (buffer[0] == '$' && buffer[1] == 'A' && buffer[2] == 'M' && buffer[3] == 'J')
        {
            if (state != 0)
            {
                cout << "[-] Invalid Control Card! (AMJ) : Previous program is not ended" << endl;
                continue;
            }

            bool valid = true;
            for (int i = 4;  i < 4+12; i++)
            {
                if (buffer[i] > '9' || buffer[i] < '0')
                {
                    cout << "[-] Invalid Control Card! (AMJ)" << endl;
                    valid = false;
                    break;
                }
            }
            if (!valid) break;

            int pid = readIntFromWord(&buffer[4]);
            int ttl = readIntFromWord(&buffer[8]);
            int tll = readIntFromWord(&buffer[12]);
            init(pid, ttl, tll);
            state = 1;
        }
        else if (buffer[0] == '$' && buffer[1] == 'D' && buffer[2] == 'T' && buffer[3] == 'A')
        {
            if (state != 1)
            {
                cout << "[-] Invalid Control Card! (DTA) : program not initlized or Previous program is still executing" << endl;
                continue;
            }
            state = 2;

            IC = 00;
            for (int i = 0; i < 4; i++) M[CS][i] = '0';
            Execute();
        }
        else if (buffer[0] == '$' && buffer[1] == 'E' && buffer[2] == 'N' && buffer[3] == 'D')
        {
            if (state != 2)
            {
                cout << "[-] Invalid Control Card! (END) : program not initlized" << endl;
                continue;
            }
            cout<<endl<< "[+] Exiting Program..." << endl;
            x=0;
            state = 0;
        }
        else
        {
            if (state != 1)
            {
                cout<<endl<< "[-] Invalid Card! : program not initlized" << endl;
                continue;
            }

            cout<<endl<<"[+] Loading Program card..."<<endl;
            int k = 0;

            int address = resolveValidPageFault(CS, true, x % 100);
            
            for(; x < (MEMS - REGS); ++x)
            {
                for(int j = 0 ; j < 4; ++j)
                {
                    M[address + (x % 10)][j] = buffer[k];
                    k++;
                }
                
                if (k == 40 || buffer[k] == ' ' || buffer[k] == '\n')
                {
                    x++;
                    break;
                }
                
            }

            if ((int)(x / 100) != prvS)
            {
                writeIntToWord(readIntFromWord(M[CS]) + 1, M[CS]);
                prvS += 1;
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