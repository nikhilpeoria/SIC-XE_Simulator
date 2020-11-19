#include <iostream>
#include <stdbool.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <iterator>

using namespace std;

// -----Macro Definitions-----

#define MAX_STRING_BUFF_SIZE 128
#define ENABLE_CLI 1
#define INSTRUCTION_FMT_1_BYTE_LEN 1
#define INSTRUCTION_FMT_2_BYTE_LEN 2
#define INSTRUCTION_FMT_3_BYTE_LEN 3
#define INSTRUCTION_FMT_4_BYTE_LEN 4
#define MAX_TEXT_RECORD_COL_LEN 69

// -----Error Codes-----

int ERR_INVALID_ARGS = 1;
int ERR_INVALID_INSTRUCTIONS_FILE = 2;
int ERR_INVALID_REGISTERS_FILE = 3;
int ERR_INVALID_INPUT_FILE = 4;
int ERR_INVALID_ASSEMBLER_DIRECTIVE_FILE = 5;
int ERR_INVALID_OPCODE = 6;
int ERR_INVALID_NUM_OF_BYTES = 7;
int ERR_INVALID_ASSEMBLER_DIRECTIVE_OPERAND = 8;
int ERR_INVALID_ENCRYPTION_KEY = 9;
int SUCCESS = 0;

// -----Global Map Variables-----

// OPTAB
/*
* string - Opcode Mnemonic
* int - Instruction Format Number
* string - Opcode Number
*/
unordered_map<string, pair<int, string>> OPTAB;

// REGTAB
/*
* string - Register Name
* int - Register Number
*/
unordered_map<string, int> REGTAB;

// ADTAB
/*
* string - Assembler Directive Name
* int - Assembler Directive Number
*/
unordered_map<string, int> ADTAB;

// SYMTAB
/*
* string - Label Name
* int - Address
*/
unordered_map<string, int> SYMTAB;

// -----Global Data Structures-----

struct cli_data_s
{
    char assembler_directives_filename[MAX_STRING_BUFF_SIZE];
    char input_filename[MAX_STRING_BUFF_SIZE];
    char instructions_filename[MAX_STRING_BUFF_SIZE];
    char registers_filename[MAX_STRING_BUFF_SIZE];
    char object_code_filename[MAX_STRING_BUFF_SIZE];
    int encryption_key;
};

struct flags_s
{
    uint8_t e : 1;
    uint8_t p : 1;
    uint8_t b : 1;
    uint8_t x : 1;
    uint8_t i : 1;
    uint8_t n : 1;
    uint8_t res2 : 1;
    uint8_t res1 : 1;
};

union flags_u
{
    uint8_t flag;
    flags_s flag_bits;
};

struct machine_code_s
{
    uint32_t fourth_byte : 8; // Disp (8-bits) (Format-4 only)
    uint32_t third_byte : 8;  // Disp (8-bits)
    uint32_t second_byte : 8; // Contains xbpe (4-bits) + Disp(4-bits)
    uint32_t first_byte : 8;  // Contains opcode (6-bits) + ni flags (2-bits)
};

struct format_s // May not require
{
    uint32_t address : 20;
    uint32_t flags : 6;
    uint32_t opcode : 6;
};

union machine_code_u
{

    uint32_t machine_code;
    machine_code_s bytes;
};

struct instruction_data_s
{
    string label;
    string opcode;
    string operand;
    uint32_t instr_address;
    flags_u addressing_flags;
    bool is_machine_code_required;
    int machine_bytes;
    uint32_t final_machine_code;
};

// -----Global Variables-----

int LOCCTR;
instruction_data_s temp_instruction_data; // Used for Pass 1
cli_data_s cli_data;
vector<instruction_data_s> inst_v;
machine_code_u temp_machine_code;
flags_u temp_flags;
int base;
char mod_line[500] = {0};

// -----Function Declarations-----

void init_global_variables();
void cli__main_menu();
char *getCmdOption(char **begin, char **end, const string &option);
bool cmdOptionExists(char **begin, char **end, const string &option);
bool validate_cli_encryption_key(string);
int cli__run_program();
int cli__show_file_contents(char *);

// Utility Functions

int hextoint(string hexstring);

string convert_int_to_hex_string(int);

// Intialization Functions

void fill_optab();
void fill_regtab();
void fill_adtab();
void load__registers_from_file();
void load__instructions_from_file();

// Pass 1 Assembly Functions

void assign_address();
void insert_in_symtab();
int parse_sample_program_into_data_structure();
int pass_1_assembly();
int check_validity();
int validate_instruction_arguments(char *);
int validate_opcode(char *);
int determine_format_type();
int increment_locctr();
int get_BYTE_constant_byte_len();

// Pass 2 Assembly Functions

void generate_final_object_code(std::vector<instruction_data_s>::iterator);
void load_immediate(std::vector<instruction_data_s>::iterator);
void generate_final_machine_code(std::vector<instruction_data_s>::iterator);
void load_constant(std::vector<instruction_data_s>::iterator);
void recalculate_base_displacement(std::vector<instruction_data_s>::iterator);
int calculate_displacement(std::vector<instruction_data_s>::iterator);
void decode_operand_fmt_2(std::vector<instruction_data_s>::iterator);
void set_flags(std::vector<instruction_data_s>::iterator, int);
void look_up_opcode(std::vector<instruction_data_s>::iterator);
void pass_2_assembly();

// -----Function Definitions-----

// Utility Functions

int hextoint(string hexstring)
{
    int number = (int)strtol(hexstring.c_str(), NULL, 16);
    return number;
}

string convert_int_to_hex_string(int hex_num)
{
    stringstream ss;
    ss << hex << uppercase << hex_num;
    return ss.str();
}

// Initialization Functions

void init_global_variables()
{
    LOCCTR = 0;
    memset(&temp_instruction_data, 0, sizeof(temp_instruction_data));
    memset(&cli_data, 0, sizeof(cli_data));
    inst_v.clear();
}

void fill_optab()
{
    FILE *fp = fopen(cli_data.instructions_filename, "r");
    char opcode_line[MAX_STRING_BUFF_SIZE] = {0};

    while (fgets(opcode_line, sizeof(opcode_line), fp))
    {
        char opcode_name_char[10], opcode_hex_char[10];
        int opcode_format;
        string opcode_hex_string, opcode_name_string;
        stringstream opcode_hex_ss, opcode_name_ss;

        sscanf(opcode_line, "%s %d %s", opcode_name_char, &opcode_format, opcode_hex_char);

        opcode_name_ss << opcode_name_char;
        opcode_hex_ss << opcode_hex_char;

        opcode_hex_string = opcode_hex_ss.str();
        opcode_name_string = opcode_name_ss.str();

        OPTAB[opcode_name_string] = make_pair(opcode_format, opcode_hex_string);
        cout << opcode_name_string << " " << opcode_format << " " << opcode_hex_string << endl;
    }
    fclose(fp);
}

void fill_regtab()
{
    FILE *fp = fopen(cli_data.registers_filename, "r");
    char opcode_line[MAX_STRING_BUFF_SIZE] = {0};

    while (fgets(opcode_line, sizeof(opcode_line), fp))
    {
        char reg_name[10] = {0};
        int reg_num = 0;

        sscanf(opcode_line, "%s %d", reg_name, &reg_num);

        REGTAB[reg_name] = reg_num;
        cout << reg_name << " " << reg_num << endl;
    }
    fclose(fp);
}

void fill_adtab()
{
    FILE *fp = fopen(cli_data.assembler_directives_filename, "r");
    char ad_line[MAX_STRING_BUFF_SIZE] = {0};

    while (fgets(ad_line, sizeof(ad_line), fp))
    {
        char ad_name[10] = {0};
        int ad_num = 0;

        sscanf(ad_line, "%s %d", ad_name, &ad_num);

        ADTAB[ad_name] = ad_num;
        cout << ad_name << " " << ad_num << endl;
    }
    fclose(fp);
}

// Pass 1 Assembly Functions

int get_BYTE_constant_byte_len()
{
    if (temp_instruction_data.operand[0] == 'C' && temp_instruction_data.operand[1] == '\'')
    {
        return (temp_instruction_data.operand.length() - 3);
    }
    else if (temp_instruction_data.operand[0] == 'X' && temp_instruction_data.operand[1] == '\'')
    {
        return ((temp_instruction_data.operand.length() - 3) / 2);
    }
    else
    {
        cout << "Error: Invalid Operand(s) found in assembler directive." << endl;
        return ERR_INVALID_ASSEMBLER_DIRECTIVE_OPERAND;
    }
}

int validate_instruction_arguments(char *opcode_line)
{
    char label[20] = {0}, opcode[20] = {0}, operand[20] = {0};
    string strFromChar, word;
    int number_arg = 0, i = 0;
    bool no_label = false;

    strFromChar.append(opcode_line);
    istringstream ss(strFromChar);

    if (opcode_line[0] == ' ')
    {
        no_label = true;
        number_arg++;
    }

    string arr_instruction[3];
    while (ss >> word)
    {
        if (no_label && i == 0)
        {
            arr_instruction[0] = "_";  // Label
            arr_instruction[1] = word; // Opcode
            i += 2;
            continue;
        }
        arr_instruction[i] = word;
        i++;
        number_arg++;
    }

    if (number_arg >= 4)
    {
        return ERR_INVALID_ARGS;
    }

    temp_instruction_data.label = arr_instruction[0];
    temp_instruction_data.opcode = arr_instruction[1];
    temp_instruction_data.operand = arr_instruction[2];

    return SUCCESS;
}

int validate_opcode(char *opcode_line)
{
    int ret_status = SUCCESS;
    string temp_opcode;
    temp_opcode = temp_instruction_data.opcode;
    if (temp_instruction_data.opcode[0] == '+')
    {
        temp_opcode = &temp_instruction_data.opcode[1];
    }

    if (ADTAB.find(temp_opcode) != ADTAB.end())
    {
        temp_instruction_data.is_machine_code_required = false;
        if (temp_instruction_data.opcode == "BYTE")
        {
            temp_instruction_data.is_machine_code_required = true;
        }
    }
    else if (OPTAB.find(temp_opcode) != OPTAB.end())
    {
        temp_instruction_data.is_machine_code_required = true;
    }
    else
    {
        ret_status = ERR_INVALID_OPCODE;
        cout << "Invalid Opcode: " << temp_instruction_data.opcode << ". Error Code: " << ret_status << endl;
    }
    return ret_status;
}

int check_validity(char *opcode_line)
{
    int err_code = validate_instruction_arguments(opcode_line);
    if (err_code == SUCCESS)
    {
        err_code = validate_opcode(opcode_line);
        return err_code;
    }
    return ERR_INVALID_ARGS;
}

// Addressing Function Definitions

int determine_format_type()
{
    int number_of_bytes = 0;
    string temp_opcode = temp_instruction_data.opcode;

    /* We calculate the machine code for instruction lines  *
     * with valid opcodes or BYTE assembler directive only */

    if (temp_instruction_data.is_machine_code_required == true)
    {
        if (temp_instruction_data.opcode[0] == '+')
        {
            temp_opcode = &temp_instruction_data.opcode[1];
        }

        pair<int, string> temp = OPTAB[temp_opcode];
        number_of_bytes = temp.first;

        // cout << "DEBUG 2 " << temp_instruction_data.opcode << " " << temp.first << " " << temp.second << endl;
        if ((temp_instruction_data.opcode[0] == '+') && (number_of_bytes == INSTRUCTION_FMT_3_BYTE_LEN))
        {
            // cout << "DEBUG 1" << temp_instruction_data.opcode << endl;
            number_of_bytes = INSTRUCTION_FMT_4_BYTE_LEN;
        }
        else if (temp_instruction_data.opcode == "BYTE")
        {
            number_of_bytes = get_BYTE_constant_byte_len();
            if (number_of_bytes == ERR_INVALID_ASSEMBLER_DIRECTIVE_OPERAND)
            {
                return ERR_INVALID_NUM_OF_BYTES;
            }
        }
    }
    else
    {
        if (temp_instruction_data.opcode == "RESW")
        {
            number_of_bytes = 3 * stoi(temp_instruction_data.operand);
        }
        else if (temp_instruction_data.opcode == "RESB")
        {
            number_of_bytes = 1 * stoi(temp_instruction_data.operand);
        }
        else if (temp_instruction_data.opcode == "START" || temp_instruction_data.opcode == "END" || temp_instruction_data.opcode == "BASE")
        {
            number_of_bytes = 0;
        }
        else if (temp_instruction_data.opcode == "WORD")
        {
            number_of_bytes = 3;
        }
        else
        {
            cout << "ERROR: Could not determine the Assembler Directive." << endl;
        }
    }
    temp_instruction_data.machine_bytes = number_of_bytes;
    return number_of_bytes;
}

int increment_locctr()
{
    // cout << convert_int_to_hex_string(LOCCTR) << "  " << temp_instruction_data.label << "  " << temp_instruction_data.opcode << "  " << temp_instruction_data.operand << endl;
    int num_of_bytes = determine_format_type();

    if (num_of_bytes == ERR_INVALID_NUM_OF_BYTES)
    {
        return ERR_INVALID_NUM_OF_BYTES;
    }

    LOCCTR += num_of_bytes;
    return SUCCESS;
}

void assign_address()
{
    // If START is not present, initialize the address to 0
    temp_instruction_data.instr_address = 0;

    if (temp_instruction_data.opcode == "START")
    {
        LOCCTR = stoi(temp_instruction_data.operand);
        temp_instruction_data.instr_address = LOCCTR;
    }
    temp_instruction_data.instr_address = LOCCTR;
}

void insert_in_symtab()
{
    if (temp_instruction_data.label != "_" && temp_instruction_data.label != " " && temp_instruction_data.label != "")
    {
        SYMTAB[temp_instruction_data.label] = LOCCTR;
        cout << temp_instruction_data.label << "  " << convert_int_to_hex_string(SYMTAB[temp_instruction_data.label]) << endl;
    }
}

int parse_sample_program_into_data_structure()
{
    int ret_status = SUCCESS, line_num;
    FILE *fp = fopen(cli_data.input_filename, "r");
    char opcode_line[100];
    if (fp != NULL)
    {
        memset(&temp_instruction_data, 0, sizeof(temp_instruction_data));
        while (fgets(opcode_line, sizeof(opcode_line), fp))
        {
            if (check_validity(opcode_line) == SUCCESS)
            {
                assign_address();
                insert_in_symtab();
                ret_status = increment_locctr();
                if (ret_status != SUCCESS)
                {
                    cout << "Error at line " << line_num << ". Return Code: " << ret_status << endl;
                }
                inst_v.push_back(temp_instruction_data);
            } // TODO: Handle error validity case
            line_num++;
        }
        fclose(fp);
    }
    else
    {
        cout << "Error opening the Input file: " << cli_data.input_filename << endl;
        return ERR_INVALID_INPUT_FILE;
    }
    return SUCCESS;
}

int pass_1_assembly()
{
    int ret_status = parse_sample_program_into_data_structure();
    return ret_status;
}

// Pass 2 Assembly Function Definitions

void generate_final_object_code(std::vector<instruction_data_s>::iterator it)
{
    string temp_operand = (*it).operand;
    string temp_opcode = (*it).opcode;
    string temp_label = (*it).label;
    static int start_address = 0;
    bool temp_code_required = (*it).is_machine_code_required;
    FILE *fp = fopen(cli_data.object_code_filename, "a");
    if (temp_opcode == "START")
    {
        // Generate Header record
        start_address = (*it).instr_address;
        char header_line[100] = {0};
        string program_name = temp_label;
        string starting_address = convert_int_to_hex_string(start_address);
        string length_of_object_program = convert_int_to_hex_string(LOCCTR);
        strcat(header_line, "H^");
        strcat(header_line, program_name.c_str());
        strcat(header_line, "^");
        for (int i = 0; i < (6 - starting_address.length()); i++) // Zero padding
        {
            strcat(header_line, "0");
        }
        strcat(header_line, starting_address.c_str());
        strcat(header_line, "^");
        for (int i = 0; i < (6 - length_of_object_program.length()); i++) // Zero padding
        {
            strcat(header_line, "0");
        }
        strcat(header_line, length_of_object_program.c_str());
        strcat(header_line, "\0");
        strcat(header_line, "\n");
        fputs(header_line, fp);
    }

    else if (temp_opcode != "START" && temp_opcode != "END" && temp_code_required)
    {
        // Generate Text Record
        static int column_index = 0;
        static int number_of_bytes_in_record = 0;
        static int current_record_address = start_address; // Col 2-7
        static int prev_record_address = 0;
        if (column_index == 0)
        {
            fputs("T^", fp);
            column_index = 2;
        }
        if (column_index >= 2 && column_index <= 7)
        {
            current_record_address = (*it).instr_address;
            string record_starting_address = convert_int_to_hex_string(current_record_address);
            for (int i = 0; i < (6 - record_starting_address.length()); i++) // Zero padding
            {
                fputs("0", fp);
            }
            fputs(record_starting_address.c_str(), fp);
            fputs("^", fp);
            column_index = 8;
        }
        if (column_index >= 8 && column_index <= 9)
        {
            vector<instruction_data_s>::iterator nx = inst_v.end();
            while ((*nx).is_machine_code_required != true)
            {
                nx = prev(nx, 2);
            }
            int length_of_object_code = 0;
            if (current_record_address + 0x1D <= LOCCTR - (*nx).machine_bytes)
            {
                length_of_object_code = 0x1d;
            }
            else
            {
                length_of_object_code = LOCCTR - (*nx).machine_bytes - prev_record_address;
            }

            fputs(convert_int_to_hex_string(length_of_object_code).c_str(), fp);
            fputs("^", fp);
            column_index = 10;
        }
        if (column_index >= 10 && column_index <= MAX_TEXT_RECORD_COL_LEN)
        {
            string temp_m_code = convert_int_to_hex_string((*it).final_machine_code); // TODO: Check for unsigned issues for 4 bytes
            int zero_padding = ((*it).machine_bytes * 2) - temp_m_code.length();
            for (int i = 0; i < zero_padding; i++)
            {
                fputs("0", fp);
                column_index++;
            }
            fputs(temp_m_code.c_str(), fp);
            column_index += temp_m_code.length();
            // cout << "COLUMN INDEX: " << dec << column_index << " Machine Code: " << temp_m_code << endl;
            // cout << "DEBUG 7: " << hex << ((*nx).final_machine_code) << endl;

            if ((column_index + 2) > MAX_TEXT_RECORD_COL_LEN) // (column_index + convert_int_to_hex_string(inst_v[vect_i + 1].final_machine_code).length()) > MAX_TEXT_RECORD_COL_LEN)
            {
                fputs("\n", fp);
                column_index = 0; // Start new text record
                number_of_bytes_in_record = 0;
                prev_record_address = (*it).instr_address;
            }
            else
            {
                fputs("^", fp);
            }

            if (temp_opcode != "START" && !(temp_flags.flag_bits.p || temp_flags.flag_bits.b) && temp_flags.flag_bits.n && (temp_flags.flag_bits.i) && temp_opcode != "END" && temp_code_required && (*it).machine_bytes == 4)
            {
                // Generate Modification record
                int curr_addr = (*it).instr_address;
                string starting_location_of_address_field = convert_int_to_hex_string(curr_addr + 1);
                string length_of_the_address_field = "05";
                strcat(mod_line, "M^");
                for (int i = 0; i < (6 - starting_location_of_address_field.length()); i++) // Zero padding
                {
                    strcat(mod_line, "0");
                }
                strcat(mod_line, starting_location_of_address_field.c_str());
                strcat(mod_line, "^");
                strcat(mod_line, length_of_the_address_field.c_str());
                strcat(mod_line, "\n");
            }
        }
    }

    else if (temp_opcode == "END")
    {
        // Write Modification record
        fputs("\n", fp);
        fputs(mod_line, fp);
        // Generate End record
        char end_line[100] = {0};
        string starting_address = convert_int_to_hex_string(start_address);
        strcat(end_line, "E^");
        for (int i = 0; i < (6 - starting_address.length()); i++) // Zero padding
        {
            strcat(end_line, "0");
        }
        strcat(end_line, starting_address.c_str());
        strcat(end_line, "\0");
        strcat(end_line, "\n");
        fputs(end_line, fp);
    }
    else
    {
        // Assembler Directive (Skip)
    }

    fclose(fp);
}

void generate_final_machine_code(std::vector<instruction_data_s>::iterator it)
{
    int num_bytes = (*it).machine_bytes;
    switch (num_bytes)
    {
    case 1:
        (*it).final_machine_code = (temp_machine_code.machine_code >> 24);
        break;
    case 2:
        (*it).final_machine_code = (temp_machine_code.machine_code >> 16);
        break;
    case 3:
        (*it).final_machine_code = (temp_machine_code.machine_code >> 8);
        break;
    case 4:
        (*it).final_machine_code = (temp_machine_code.machine_code);
        break;
    default:
        // Assembler Directive
        break;
    }
}

void load_immediate(std::vector<instruction_data_s>::iterator it)
{
    string temp_operand = (*it).operand;
    if (temp_operand[0] == '#') // Gets rid of special chars
    {
        temp_operand = &temp_operand[1];
    }
    if (SYMTAB.find(temp_operand) == SYMTAB.end())
    {
        uint32_t immediate_operand = stoi(temp_operand);

        // cout << "Immediate operand is: " << hex << immediate_operand << endl;
        if ((*it).machine_bytes == 3)
        {
            temp_machine_code.machine_code |= ((immediate_operand << 8) & 0xFFF00);
        }
        if ((*it).machine_bytes == 4)
        {
            temp_machine_code.machine_code |= (immediate_operand & 0xFFFFF);
        }
    }
}

void load_constant(std::vector<instruction_data_s>::iterator it)
{
    string temp_operand = (*it).operand;
    int num_bytes = (*it).machine_bytes;
    if (temp_operand[0] == 'C')
    {
        int temp_bytes[num_bytes];
        temp_operand = temp_operand.substr(2, ((temp_operand.find("\'", 2)) - 2)); // Removes all characters after quote
        // cout << "Char const is: " << temp_operand << " Size: " << num_bytes << endl;
        memset(&temp_bytes, 0, sizeof(temp_bytes));
        for (int j = 0; j < num_bytes; j++)
        {
            temp_bytes[j] = temp_operand[j];
            // cout << "Temp byte: " << hex << temp_bytes[j] << " " << j << endl;
        }
        switch (num_bytes)
        {
        case 1:
            temp_machine_code.bytes.first_byte = temp_bytes[0];
            break;
        case 2:
            temp_machine_code.bytes.first_byte = temp_bytes[0];
            temp_machine_code.bytes.second_byte = temp_bytes[1];
            break;
        case 3:
            temp_machine_code.bytes.first_byte = temp_bytes[0];
            temp_machine_code.bytes.second_byte = temp_bytes[1];
            temp_machine_code.bytes.third_byte = temp_bytes[2];
            break;
        default:
            cout << "Hit default case in switch" << endl;
        }
    }
    else if (temp_operand[0] == 'X')
    {
        temp_operand = temp_operand.substr(2, ((temp_operand.find("\'", 2)) - 2)); // Removes all characters after quote
        int temp_integer_constant = hextoint(temp_operand);
        switch (num_bytes)
        {
        case 1:
            temp_machine_code.machine_code = (temp_integer_constant << 24);
            break;
        case 2:
            temp_machine_code.machine_code = (temp_integer_constant << 16);
            break;
        case 3:
            temp_machine_code.machine_code = (temp_integer_constant << 8);
            break;
        default:
            cout << "Hit default case in switch" << endl;
        }
    }
    else
    {
        cout << "Invalid Character constant" << endl;
    }
}

void recalculate_base_displacement(std::vector<instruction_data_s>::iterator it)
{
    int new_disp = 0;
    string temp_operand = (*it).operand;
    if (temp_operand[0] == '#' || temp_operand[0] == '@') // Gets rid of special chars
    {
        temp_operand = &temp_operand[1];
    }
    temp_operand = temp_operand.substr(0, temp_operand.find(",", 0)); // Removes all characters after comma
    if (SYMTAB.find(temp_operand) != SYMTAB.end())
    {
        uint16_t target_address = SYMTAB[temp_operand];
        // cout << "Inst address is: " << (*it).instr_address << " Format " << (*it).machine_bytes << "Target address: " << target_address << endl;
        new_disp = uint16_t(target_address - base);
    }
    temp_machine_code.machine_code &= 0xFFF000FF;                  // Clearing old displacement
    temp_machine_code.machine_code |= ((new_disp << 8) & 0xFFF00); // Also checks if disp is within 0 to 4095
}

int calculate_displacement(std::vector<instruction_data_s>::iterator it)
{
    uint32_t disp = 0;
    string temp_operand = (*it).operand;
    if (temp_operand[0] == '#' || temp_operand[0] == '@') // Gets rid of special chars
    {
        temp_operand = &temp_operand[1];
    }
    temp_operand = temp_operand.substr(0, temp_operand.find(",", 0)); // Removes all characters after comma
    if ((*it).machine_bytes == 3)                                     // Calculating Format 3 displacement
    {
        if (SYMTAB.find(temp_operand) != SYMTAB.end())
        {
            uint16_t target_address = SYMTAB[temp_operand];
            uint16_t next_instruction_address = (*it).instr_address + (*it).machine_bytes;
            // cout << "Inst address is: " << (*it).instr_address << " Format " << (*it).machine_bytes << "Target address: " << target_address << endl;
            disp = uint16_t(target_address - next_instruction_address);
        }
        temp_machine_code.machine_code |= ((disp << 8) & 0xFFF00);
    }
    if ((*it).machine_bytes == 4)
    {
        if (SYMTAB.find(temp_operand) != SYMTAB.end()) // Calculating Format 4 displacement
        {
            disp = SYMTAB[temp_operand]; // Looking up label in symtab
        }
        else
        {
            disp = stoi(temp_operand);
        }
        temp_machine_code.machine_code |= (disp & 0xFFFFF);
    }
    return disp;
}

// Register decoding for instruction format-2
void decode_operand_fmt_2(std::vector<instruction_data_s>::iterator it)
{
    string temp_operand = (*it).operand, temp_operands_string, temp_operand_v[2];
    uint32_t operand_value = 0;
    stringstream ss(temp_operand), ss1;
    int i = 0;
    while (ss.good())
    {
        string substr;
        getline(ss, substr, ',');
        temp_operand_v[i] = substr;
        i++;
    }

    for (size_t i = 0; i < 2; i++)
    {
        temp_operands_string += convert_int_to_hex_string(REGTAB[temp_operand_v[i]]);
    }
    // cout << "Operand String is: " << temp_operands_string << endl;

    ss1 << hex << temp_operands_string;

    ss1 >> operand_value;

    temp_machine_code.bytes.second_byte = operand_value;
}

void look_up_opcode(std::vector<instruction_data_s>::iterator it)
{
    string temp_opcode = (*it).opcode;
    // cout << "Opcode is " << temp_opcode << endl;
    // stringstream string_stream(temp_opcode); // Ignore
    int opcode_value = 0;
    if ((*it).opcode[0] == '+')
    {
        temp_opcode = &temp_opcode[1];
    }
    string machine_equivalent_hex_string = (OPTAB[temp_opcode].second);
    // cout << "Looked up OPTAB and found: " << machine_equivalent_hex_string << endl;
    // string_stream << hex << machine_equivalent_hex_string; // <- hex_string B4 is 2 bytes // Ignore
    // string_stream >> opcode_value;                         // opcode_value <- 2 bytes <- B400 // Ignore
    opcode_value = hextoint(machine_equivalent_hex_string);
    // cout << "Final opcode value (hex_int): " << hex << (opcode_value) << endl;
    temp_machine_code.bytes.first_byte = opcode_value;
}

void set_flags(std::vector<instruction_data_s>::iterator it, int disp)
{
    string temp_operand = (*it).operand;
    string temp_opcode = (*it).opcode;
    temp_flags.flag = 0;
    if (temp_operand[0] == '#') // Checking for immediate addressing
    {
        // cout << "Operand is: " << temp_operand << endl;
        temp_flags.flag_bits.i = 1;
        temp_flags.flag_bits.n = 0;
        load_immediate(it);
    }
    else if (temp_operand[0] == '@') // Checking for indirect addressing
    {
        temp_flags.flag_bits.i = 0;
        temp_flags.flag_bits.n = 1;
    }
    else
    {
        // temp_flags.flag_bits.i = (temp_machine_code.bytes.first_byte & 1); // Assigning simple addressing
        // temp_flags.flag_bits.n = (temp_machine_code.bytes.first_byte & 2);
        temp_flags.flag_bits.i = 1; // Assigning simple addressing
        temp_flags.flag_bits.n = 1;
    }

    if (temp_operand.find(",X") != temp_operand.npos) // Checking for x (Indexed Mode)
    {
        temp_flags.flag_bits.x = 1;
    }
    if ((*it).machine_bytes == 4 && temp_opcode[0] == '+') // Checking for e
    {
        temp_flags.flag_bits.e = 1;
    }
    // Base-relative 0 to 4095
    // PC-relative -2048 to 2047
    if (disp != 0 && temp_flags.flag_bits.e != 1)
    {
        int16_t signed_disp = disp;                      // Handling negative displacement
        if (signed_disp >= -2048 && signed_disp <= 2047) // PC-relative
        {
            temp_flags.flag_bits.b = 0;
            temp_flags.flag_bits.p = 1;
        }
        else // Base-relative displacement
        {
            temp_flags.flag_bits.b = 1;
            temp_flags.flag_bits.p = 0;
            recalculate_base_displacement(it);
        }
        // else
        // {
        //     temp_flags.flag_bits.b = 0;
        //     temp_flags.flag_bits.p = 0;
        // }
    }
    // cout << "Displacement is: " << hex << disp << endl;
    temp_machine_code.machine_code |= ((temp_flags.flag << 20) & 0x03F00000);
}

void pass_2_assembly()
{
    for (auto it = inst_v.begin(); it < inst_v.end(); it++)
    {
        int disp = 0;
        memset(&temp_machine_code, 0, sizeof(temp_machine_code));
        // Exceptions
        if ((*it).machine_bytes == 0) // Handling Base relative exception
        {
            if ((*it).opcode == "BASE")
            {
                base = SYMTAB[(*it).operand];
            }
        }
        if ((*it).opcode == "BYTE")
        {
            load_constant(it);
        }
        // Format 1-4 Operations
        if ((*it).machine_bytes != 0 && (*it).opcode != "BYTE" && (*it).is_machine_code_required == true) // Ignore for Assembler Directives
        {
            look_up_opcode(it);           // Format 1 handled here
            if ((*it).machine_bytes == 2) // Format 2 handled here
            {
                decode_operand_fmt_2(it);
            }

            if ((*it).machine_bytes == 3 || (*it).machine_bytes == 4) // Format 3&4 handled here
            {
                disp = calculate_displacement(it);
                set_flags(it, disp);
            }
        }
        generate_final_machine_code(it);
        // cout << (*it).opcode << " First byte " << hex << temp_machine_code.bytes.first_byte << " Second byte " << hex << temp_machine_code.bytes.second_byte << " Third byte " << hex << temp_machine_code.bytes.third_byte << " Fourth byte " << hex << temp_machine_code.bytes.fourth_byte << endl;
        cout << hex << uppercase << (*it).instr_address << " " << (*it).label << " " << (*it).opcode << " " << (*it).operand << " " << hex << uppercase << (*it).final_machine_code << endl;
        generate_final_object_code(it);
    }
}

// CLI Function Definitions

char *getCmdOption(char **begin, char **end, const string &option)
{
    char **itr = find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char **begin, char **end, const string &option)
{
    return find(begin, end, option) != end;
}

void display_title()
{
    cout << "  _____ _____ _____   __   ________                                    _     _              ____  _                 _       _              " << endl;
    cout << " / ____|_   _/ ____|  \" \" / /  ____|      /\"                          | |   | |            / ___|(_)               | |     | |             " << endl;
    cout << "| (___   | | | |       \" V /| |__        /  \"   ___ ___  ___ _ __ ___ | |__ | | ___ _ _   | (___  _ _ __ ___  _   _| | __ _| |_   ___  _ __  " << endl;
    cout << " \"___ \"  | | | |        > < |  __|      / /\" \" / __/ __|/ _ \" '_ ` _ \"| '_ \"| |/ _ \" '__|  \\___ \"| | '_ ` _ \"| | | | |/ _` | __/ / _ \"| '__| " << endl;
    cout << " ____) |_| |_| |___    / . \"| |____    / ____ \\__ \"__ \"  __/ | | | | |  |_) | |  __/ |    ____)  | | | | | | | |_| | | (_| | |  | (_) | |    " << endl;
    cout << "|_____/|_____|\\____|  /_/ \"_\"______|  /_/    \"_\"___/___/\"__|_| |_| |_|_.___/|_|___||_|   |_____/ |_|_| |_| |_|\"__,_|_|\"__,_|\"__\" \\___/|_|    " << endl;
}

void display_help()
{
    cout << endl
         << R"(---------ARGUMENT HELP SECTION---------)" << endl
         << endl;
    cout << R"( - Input filename        -> Input Assembly filename)" << endl;
    cout << R"( - Machine code filename -> Output Machine code filename)" << endl;
    cout << R"( - Object code filename  -> Output Object code filename)" << endl;
    cout << R"( - Machine and Object code files encryption [y/n]: -> To select encryption)" << endl;
}

bool validate_cli_encryption_key(string encr_key)
{
    bool is_key_valid = true;
    int key_len = encr_key.length();

    if (key_len != 6)
    {
        is_key_valid = false;
    }

    for (int i = 0; i < key_len; i++)
        if (isdigit(encr_key[i]) == false)
            is_key_valid = false;

    if (is_key_valid == true)
    {
        cli_data.encryption_key = stoi(encr_key);
    }
    return is_key_valid;
}

void cli__main_menu()
{
#if ENABLE_CLI == 1
    int cli_user_choice = 0, ret_status = SUCCESS;
    char filename[MAX_STRING_BUFF_SIZE] = {0};

    display_title();

    while (cli_user_choice <= 3)
    {
        init_global_variables();
        cout << endl;
        cout << "---------MENU---------\n";
        cout << "1. Begin Assembler Processing\n";
        cout << "2. Show Instructions\n";
        cout << "3. Show Registers\n";
        cout << "4. Quit\n"
             << flush;
        cout << "\n> Enter your choice: ";

        cin >> cli_user_choice;
        if (cli_user_choice >= 5 || cli_user_choice <= 0)
        {
            cout << endl
                 << "Invalid user choice. Exiting the program.";
            exit(0);
        }
        switch (cli_user_choice)
        {
        case 1:
            ret_status = cli__run_program();
            if (ret_status != SUCCESS)
                continue;
            break;
        case 2:
            cout << "\n> Enter Instruction File Name: ";
            cin >> filename;
            ret_status = cli__show_file_contents(filename);
            if (ret_status != SUCCESS)
                continue;
            break;
        case 3:
            cout << "\n> Enter Register File Name: ";
            cin >> filename;
            ret_status = cli__show_file_contents(filename);
            if (ret_status != SUCCESS)
                continue;
            break;
        default:
            cout << "Exiting the program.";
            exit(0);
        }
    }

// TODO: Only for debugging. Clean up on final commit.
#elif ENABLE_CLI == 0
    strcpy(cli_data.input_filename, "input_assembly_file.txt");
    strcpy(cli_data.instructions_filename, "instructions.txt");
    strcpy(cli_data.registers_filename, "registers.txt");
    strcpy(cli_data.assembler_directives_filename, "assembler_directives.txt");
    strcpy(cli_data.object_code_filename, "ocode.txt");
    cli_data.encryption_key = 0;

    cout << endl;
    cout << "------------OPTAB-------------" << endl;
    fill_optab();
    cout << "------------REGTAB-------------" << endl;
    fill_regtab();
    cout << "------------ADTAB-------------" << endl;
    fill_adtab();
    cout << "------------SYMTAB-------------" << endl;

    pass_1_assembly();
    cout << "-------------------------------" << endl;
    pass_2_assembly();
#endif
}

int cli__run_program()
{
    char encrypt_output_file_option = 'n';
    string encry_key;
    ifstream input_fp, instruction_fp, register_fp, assembler_directive_fp;

    cout << "\n> Enter Input Assembly File Name: ";
    cin >> cli_data.input_filename;
    input_fp.open((cli_data.input_filename));

    if (input_fp.fail())
    {
        cout << "\nError: Input File Not Found." << endl;
        return ERR_INVALID_INPUT_FILE;
    }

    cout << "\n> Enter Instruction File Name: ";
    cin >> cli_data.instructions_filename;
    instruction_fp.open((cli_data.instructions_filename));

    if (instruction_fp.fail())
    {
        cout << "\nError: Instruction File Not Found." << endl;
        return ERR_INVALID_INSTRUCTIONS_FILE;
    }

    cout << "\n> Enter Register File Name: ";
    cin >> cli_data.registers_filename;
    register_fp.open((cli_data.registers_filename));

    if (register_fp.fail())
    {
        cout << "\nError: Register File Not Found." << endl;
        return ERR_INVALID_REGISTERS_FILE;
    }

    cout << "\n> Enter Assembler Directive File Name: ";
    cin >> cli_data.assembler_directives_filename;
    assembler_directive_fp.open((cli_data.assembler_directives_filename));

    if (assembler_directive_fp.fail())
    {
        cout << "\nError: Assembler Directive File Not Found." << endl;
        return ERR_INVALID_ASSEMBLER_DIRECTIVE_FILE;
    }

    cout << "\n> Enter Object Code Filename: ";
    cin >> cli_data.object_code_filename;

    cout << "\n> Do you want to encrypt the Output File? [y/n]: ";
    cin >> encrypt_output_file_option;

    if (encrypt_output_file_option == 'y')
    {
        cout << "\n> Enter 6-digit Encryption Key: ";
        cin >> encry_key;
        if (validate_cli_encryption_key(encry_key) == false)
        {
            cout << "Invalid Encryption Key. Enter 6 digits only." << endl;
            return ERR_INVALID_ENCRYPTION_KEY;
        }
    }
    else
    {
        cout << "Encryption disabled.\n";
        cli_data.encryption_key = 0;
    }

    // TODO: Check if assembler program starts with START and ends with END
    cout << "------------OPTAB-------------" << endl;
    fill_optab();
    cout << "------------REGTAB-------------" << endl;
    fill_regtab();
    cout << "------------ADTAB-------------" << endl;
    fill_adtab();
    cout << "------------SYMTAB-------------" << endl;

    pass_1_assembly();
    cout << "-------------------------------" << endl;
    pass_2_assembly();

    input_fp.close();
    instruction_fp.close();
    register_fp.close();
    assembler_directive_fp.close();

    return SUCCESS;
}

int cli__show_file_contents(char *filename)
{
    string line_buff;
    ifstream file_fp;

    file_fp.open(filename);

    if (file_fp.is_open())
    {
        while (getline(file_fp, line_buff))
        {
            cout << line_buff << endl;
        }
    }
    else
    {
        cout << "Error: Unable to open file: " << filename << endl;
        return ERR_INVALID_INPUT_FILE;
    }

    file_fp.close();
    return SUCCESS;
}

int main(int argc, char *argv[])
{
    if (cmdOptionExists(argv, argv + argc, "-h"))
    {
        display_help();
    }
    else
    {
        cli__main_menu();
        cout << "LOCCTR is " << hex << LOCCTR << endl;
        return SUCCESS;
    }
}