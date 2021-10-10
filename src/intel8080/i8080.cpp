// LIBRARY-i8080

// implementation following the pdfs above, fully commented code for study purposes
// i'm doing an i8080 based implementation i'm changing some flags and registers
// implementation focused on studying how the i8080 logic works
// A sequence of instructions that get together. allow the computer perform a desired task

// References

// http://www.emulator101.com/reference/8080-by-opcode.html
// https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf
// https://pastraiser.com/cpu/i8080/i8080_opcodes.html
// https://en.wikipedia.org/wiki/FLAGS_register
// http://www.emulator101.com/full-8080-emulation.html
// https://github.com/begoon/i8080-core
// https://en.wikipedia.org/wiki/Intel_8080

// License

//  Copyright (c)  VitorMob 2021 - All rights reserved.

//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:

//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.

//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include <i8080.hpp>

#define ret PC = pop()

#define jmp PC = memory::read_memory_word(PC);

#define call      \
    push(PC + 2); \
    jmp;

#define rst(addr) \
    push(PC);     \
    PC = addr;

inline static constexpr byte_t cycles_opcode[256] = {


    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  
    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  
    4, 10, 16, 5,  5,  5,  7,  4,  4, 10, 16, 5,  5,  5,  7, 4,  
    4, 10, 13, 5,  10, 10, 10, 4,  4, 10, 13, 5,  5,  5,  7, 4,  
    
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  
    7, 7,  7,  7,  7,  7,  7,  7,  5, 5,  5,  5,  5,  5,  7, 5,  
    
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  
    
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, 
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, 
    5, 10, 10, 18, 11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11, 
    5, 10, 10, 4,  11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11  
    
};

inline static bool parity(word_t n)
{
    byte_t p_count = 0;
    for (int i = 0; i < 8; i++)
        p_count += ((n >> i) & 1);

    return (p_count & 1) == 0;
}

// instructions 
void Instructions::push(word_t data16)
{
    SP -= 2;
    memory::write_memory_word(SP, data16);
}

word_t Instructions::pop()
{
    word_t flag = memory::read_memory_word(SP);
    SP += 2;
    return flag;
}

void Instructions::add(word_t data16)
{
    this->data16 = (A + data16);
    A = this->data16 & 0xff;
    CF = (this->data16 & 0x100) != 0;
    PF = parity(A);
    SF = (A & 0x80) != 0;
    ZF = (A == 0);
    AC = this->data16 >> 7 & 0x1 ? 0 : 1;
}

void Instructions::adc(word_t data16)
{
    this->data16 = A + data16 + CF;
    A = this->data16 & 0xff;
    CF = (this->data16 & 0x100) != 0;
    PF = parity(A);
    SF = (A & 0x80) != 0;
    ZF = (A == 0);
    AC = this->data16 >> 7 & 0x1 ? 0 : 1;
}

void Instructions::dad(word_t data16)
{
    CF = ((get_hl() + data16) >> 16) & 1;
    set_hl(get_hl() + data16);
}

void Instructions::sub(word_t data16)
{
    this->data16 = A - data16;
    A = this->data16 & 0xff;
    CF = (this->data16 & 0x100) != 0;
    SF = (A & 0x80) != 0;
    PF = parity(A);
    AC = this->data16 >> 7 & 0x1 ? 0 : 1;
    ZF = (A == 0);
}

void Instructions::sbb(word_t data16)
{
    this->data16 = (A - data16) - CF;
    A = this->data16 & 0xff;
    CF = (this->data16 & 0x100) != 0;
    SF = (A & 0x80) != 0;
    PF = parity(A);
    AC = this->data16 >> 7 & 0x1 ? 0 : 1;
    ZF = (A == 0);
}

void Instructions::cmp(word_t data16)
{
    this->data16 = (A - data16);
    CF = (this->data16 & 0x100) != 0;
    SF = (this->data16 & 0x80) != 0;
    ZF = (this->data16 & 0xff) == 0;
    PF = parity(this->data16 & 0xff);
    AC = this->data16 >> 7 & 0x1 ? 0 : 1;
}

void Instructions::ana(word_t data16)
{
    AC = ((A | data16) & 0x08) != 0;
    A &= data16;
    SF = (A & 0x80) != 0;
    PF = parity(A);
    CF = 0;
    ZF = (A == 0);
}

void Instructions::ora(word_t data16)
{
    A |= data16;
    CF = 0;
    SF = (A & 0x80) != 0;
    PF = parity(A);
    AC = 0;
    ZF = (A == 0);
}

void Instructions::xra(word_t data16)
{
    A ^= data16;
    CF = 0;
    SF = (A & 0x80) != 0;
    PF = parity(A);
    AC = 0;
    ZF = (A == 0);
}

byte_t Instructions::inr(byte_t data8)
{
    this->data8 = data8 + 1;
    PF = parity(this->data8);
    SF = (this->data8 & 0x80) != 0;
    ZF = ((this->data8 & 0xff) == 0);
    AC = (this->data8 & 0xf) == 0xf;

    return this->data8;
}

byte_t Instructions::dcr(byte_t data8)
{
    this->data8 = data8 - 1;
    PF = parity(this->data8);
    SF = (this->data8 & 0x80) != 0;
    ZF = ((this->data8 & 0xff) == 0);
    AC = (this->data8 & 0xf) == 0xf;

    return this->data8;
}

void Instructions::set_bc(word_t data16)
{
    B = data16 >> 8;
    C = data16 & 0xff;
}

void Instructions::set_hl(word_t data16)
{
    H = data16 >> 8;
    L = data16 & 0xff;
}

void Instructions::set_de(word_t data16)
{
    D = data16 >> 8;
    E = data16 & 0xff;
}

word_t Instructions::get_bc()
{
    return (B << 8) | C;
}

word_t Instructions::get_hl()
{
    return (H << 8) | L;
}

word_t Instructions::get_de()
{
    return (D << 8) | E;
}

byte_t Instructions::port_in()
{
    return 0;
}

void Instructions::port_out()
{
}

void i8080::flags_init()
{
    PSW = 0, AF = 0;
    PC = 0, SP = 0;

    A = 0, C = 0, H = 0, L = 0, B = 0, D = 0, E = 0;

    AC = 0, SF = 0, CF = 0, PF = 0, ZF = 0;

    cycles = 0;
    data16 = 0;
    data8 = 0;
}

void i8080::execute_opcode(byte_t opcode)
{
    cycles += cycles_opcode[opcode];

    switch (opcode)
    {
    // undocumented
    case 0x00: // nop
    case 0x08:
    case 0x10:
    case 0x18:
    case 0x20:
    case 0x28:
    case 0x30:
    case 0x38:
        break;

    case 0x01:
        set_bc(memory::read_memory_word(PC)); // lxi bc
        PC += 2;
        break;
    case 0x02:
        memory::write_memory_byte(get_bc(), A); // stax bc
        break;
    case 0x03:
        set_bc(get_bc() + 1); // inx b
        break;
    case 0x04:
        B = inr(B);
        break;
    case 0x05:
        B = dcr(B);
        break;
    case 0x06:
        B = memory::read_memory_byte(PC++); // mvi b, d8
        break;
    case 0x07:
        CF = (A & 0x80) != 0;
        A = (A << 1) | CF;
        break;
    case 0x09:
        dad(get_bc());
        break;
    case 0x0a:
        A = memory::read_memory_byte(get_bc()); // ldax b
        break;
    case 0x0b:
        set_bc(get_bc() - 1); // dcx  bc
        break;
    case 0x0c:
        C = inr(C);
        break;
    case 0x0d:
        C = dcr(C);
        break;
    case 0x0e:
        C = memory::read_memory_byte(PC++); // mvi C, d8
        break;
    case 0x0f:
        CF = A & 0x01; // rrc
        A = (A >> 1) | (CF << 7);
        break;
    case 0x11:
        set_de(memory::read_memory_word(PC)); // lxi de
        PC += 2;
        break;
    case 0x12:
        memory::write_memory_byte(get_de(), A); // stax de
        break;
    case 0x13:
        set_de(get_de() + 1); // inx de
        break;
    case 0x14:
        D = inr(D);
        break;
    case 0x15:
        D = dcr(D);
        break;
    case 0x16:
        D = memory::read_memory_byte(PC++); // mvi d, d8
        break;
    case 0x17:
        flags = CF;
        CF = A >> 7; // ral
        A = (A << 1) | flags;
        break;
    case 0x19:
        dad(get_de());
        break;
    case 0x1a:
        A = memory::read_memory_byte(get_de()); // ldax d
        break;
    case 0x1b:
        set_de(get_de() - 1);
        break;
    case 0x1c:
        E = inr(E);
        break;
    case 0x1d:
        E = dcr(E);
        break;
    case 0x1e:
        E = memory::read_memory_byte(PC++); // mvi e, d8
        break;
    case 0x1f:
        CF = A & 0x01; // rar
        A = (A >> 1) | (CF << 7);
        break;
    case 0x21:
        data16 = memory::read_memory_word(PC); // lxi hl
        set_hl(data16);
        PC += 2;
        break;
    case 0x22:
        memory::write_memory_word(memory::read_memory_word(PC), get_hl()); // shld
        PC += 2;
        break;
    case 0x23:
        set_hl(get_hl() + 1); // inx h
        break;
    case 0x24:
        H = inr(H);
        break;
    case 0x25:
        H = dcr(H);
        break;
    case 0x26:
        H = memory::read_memory_byte(PC++); // mvi h, d8
        break;
    case 0x27: // daa
        if (AC || (A & 0xf) > 9)
        {
            A += 0x06;
        }
        if (CF || (A >> 4) > 9)
        {
            A += 0x60;
            CF = 1;
        }
        PF = parity(A);
        AC = 0;
        break;
    case 0x29:
        dad(get_hl());
        break;
    case 0x2a:
        set_hl(memory::read_memory_word(memory::read_memory_word(PC))); // lhld
        PC += 2;
        break;
    case 0x2b:
        set_hl(get_hl() - 1); // dcx h
        break;
    case 0x2c:
        L = inr(L);
        break;
    case 0x2d:
        L = dcr(L);
        break;
    case 0x2e:
        L = memory::read_memory_byte(PC++); // mvi l, d8
        break;
    case 0x2f:
        A = ~A; // cma, if A =  51H converting for aeh
        break;
    case 0x31:
        SP = memory::read_memory_word(PC); // lxi sp
        PC += 2;
        break;
    case 0x32:
        memory::write_memory_byte(memory::read_memory_word(PC), A); // sta addr
        PC += 2;
        break;
    case 0x33:
        SP++;
        break;
    case 0x34:
        memory::write_memory_word(get_hl(), inr(memory::read_memory_byte(get_hl())));
        break;
    case 0x35:
        memory::write_memory_word(get_hl(), dcr(memory::read_memory_byte(get_hl())));
        break;
    case 0x36:
        memory::write_memory_byte(get_hl(), memory::read_memory_byte(PC++)); // mvi h, d8
        break;
    case 0x37:
        CF = 1;
        break;
    case 0x39:
        dad(SP);
        break;
    case 0x3a:
        A = memory::read_memory_byte(memory::read_memory_word(PC)); // lda adr
        PC += 2;
        break;
    case 0x3b:
        SP--; // dcx sp
        break;
    case 0x3c:
        A = inr(A);
        break;
    case 0x3d:
        A = dcr(A);
        break;
    case 0x3e:
        A = memory::read_memory_byte(PC++); // mvi a, d8
        break;
    case 0x3f:
        CF = !CF; // cmc
        break;
    case 0x40:
        B = B;
        break;
    case 0x41:
        B = C;
        break;
    case 0x42:
        B = D;
        break;
    case 0x43:
        B = E;
        break;
    case 0x44:
        B = H;
        break;
    case 0x45:
        B = L;
        break;
    case 0x46:
        B = memory::read_memory_byte(get_hl());
        break;
    case 0x47:
        B = A;
        break;
    case 0x48:
        C = B;
        break;
    case 0x49:
        C = C;
        break;
    case 0x4a:
        C = D;
        break;
    case 0x4b:
        C = E;
        break;
    case 0x4c:
        C = H;
        break;
    case 0x4d:
        C = L;
        break;
    case 0x4e:
        C = memory::read_memory_byte(get_hl());
        break;
    case 0x4f:
        C = A;
        break;
    case 0x50:
        D = B;
        break;
    case 0x51:
        D = C;
        break;
    case 0x52:
        D = D;
        break;
    case 0x53:
        D = E;
        break;
    case 0x54:
        D = H;
        break;
    case 0x55:
        D = L;
        break;
    case 0x56:
        D = memory::read_memory_byte(get_hl());
        break;
    case 0x57:
        D = A;
        break;
    case 0x58:
        E = B;
        break;
    case 0x59:
        E = C;
        break;
    case 0x5a:
        E = D;
        break;
    case 0x5b:
        E = E;
        break;
    case 0x5c:
        E = H;
        break;
    case 0x5d:
        E = L;
        break;
    case 0x5e:
        E = memory::read_memory_byte(get_hl());
        break;
    case 0x5f:
        E = A;
        break;
    case 0x60:
        H = B;
        break;
    case 0x61:
        H = C;
        break;
    case 0x62:
        H = D;
        break;
    case 0x63:
        H = E;
        break;
    case 0x64:
        H = H;
        break;
    case 0x65:
        H = L;
        break;
    case 0x66:
        H = memory::read_memory_byte(get_hl());
        break;
    case 0x67:
        H = A;
        break;
    case 0x68:
        L = B;
        break;
    case 0x69:
        L = C;
        break;
    case 0x6a:
        L = D;
        break;
    case 0x6b:
        L = E;
        break;
    case 0x6c:
        L = H;
        break;
    case 0x6d:
        L = L;
        break;
    case 0x6e:
        L = memory::read_memory_byte(get_hl());
        break;
    case 0x6f:
        L = A;
        break;
    case 0x70:
        memory::write_memory_byte(get_hl(), B);
        break;
    case 0x71:
        memory::write_memory_byte(get_hl(), C);
        break;
    case 0x72:
        memory::write_memory_byte(get_hl(), D);
        break;
    case 0x73:
        memory::write_memory_byte(get_hl(), E);
        break;
    case 0x74:
        memory::write_memory_byte(get_hl(), H);
        break;
    case 0x75:
        memory::write_memory_byte(get_hl(), L);
        break;
    case 0x76:
        break; // not implemented hlt
    case 0x77:
        memory::write_memory_byte(get_hl(), A);
        break;
    case 0x78:
        A = B;
        break;
    case 0x79:
        A = C;
        break;
    case 0x7a:
        A = D;
        break;
    case 0x7b:
        A = E;
        break;
    case 0x7c:
        A = H;
        break;
    case 0x7d:
        A = L;
        break;
    case 0x7e:
        A = memory::read_memory_byte(get_hl());
        break;
    case 0x7f:
        A = A;
        break;
    case 0x80:
        add(B);
        break;
    case 0x81:
        add(C);
        break;
    case 0x82:
        add(D);
        break;
    case 0x83:
        add(E);
        break;
    case 0x84:
        add(H);
        break;
    case 0x85:
        add(L);
        break;
    case 0x86:
        add(memory::read_memory_byte(get_hl()));
        break;
    case 0x87:
        add(A);
        break;
    case 0x88:
        adc(B);
        break;
    case 0x89:
        adc(C);
        break;
    case 0x8a:
        adc(D);
        break;
    case 0x8b:
        adc(E);
        break;
    case 0x8c:
        adc(H);
        break;
    case 0x8d:
        adc(L);
        break;
    case 0x8e:
        adc(memory::read_memory_byte(get_hl()));
        break;
    case 0x8f:
        adc(A);
        break;
    case 0x90:
        sub(B);
        break;
    case 0x91:
        sub(C);
        break;
    case 0x92:
        sub(D);
        break;
    case 0x93:
        sub(E);
        break;
    case 0x94:
        sub(H);
        break;
    case 0x95:
        sub(L);
        break;
    case 0x96:
        sub(memory::read_memory_byte(get_hl()));
        break;
    case 0x97:
        sub(A);
        break;
    case 0x98:
        sbb(B);
        break;
    case 0x99:
        sbb(C);
        break;
    case 0x9a:
        sbb(D);
        break;
    case 0x9b:
        sbb(E);
        break;
    case 0x9c:
        sbb(H);
        break;
    case 0x9d:
        sbb(L);
        break;
    case 0x9e:
        sbb(memory::read_memory_byte(get_hl()));
        break;
    case 0x9f:
        sbb(A);
        break;
    case 0xa0:
        ana(B);
        break;
    case 0xa1:
        ana(C);
        break;
    case 0xa2:
        ana(D);
        break;
    case 0xa3:
        ana(E);
        break;
    case 0xa4:
        ana(H);
        break;
    case 0xa5:
        ana(L);
        break;
    case 0xa6:
        ana(memory::read_memory_byte(get_hl()));
        break;
    case 0xa7:
        ana(A);
        break;
    case 0xa8:
        xra(B);
        break;
    case 0xa9:
        xra(C);
        break;
    case 0xaa:
        xra(D);
        break;
    case 0xab:
        xra(E);
        break;
    case 0xac:
        xra(H);
        break;
    case 0xad:
        xra(L);
        break;
    case 0xae:
        xra(memory::read_memory_byte(get_hl()));
        break;
    case 0xaf:
        xra(A);
        break;
    case 0xb0:
        ora(B);
        break;
    case 0xb1:
        ora(C);
        break;
    case 0xb2:
        ora(D);
        break;
    case 0xb3:
        ora(E);
        break;
    case 0xb4:
        ora(H);
        break;
    case 0xb5:
        ora(L);
        break;
    case 0xb6:
        ora(memory::read_memory_byte(get_hl()));
        break;
    case 0xb7:
        ora(A);
        break;
    case 0xb8:
        cmp(B);
        break;
    case 0xb9:
        cmp(C);
        break;
    case 0xba:
        cmp(D);
        break;
    case 0xbb:
        cmp(E);
        break;
    case 0xbc:
        cmp(H);
        break;
    case 0xbd:
        cmp(L);
        break;
    case 0xbe:
        cmp(memory::read_memory_byte(get_hl()));
        break;
    case 0xbf:
        cmp(A);
        break;
    case 0xc0:
        if (!ZF)
            ret;
        break;
    case 0xc1:
        set_bc(pop());
        break;
    case 0xc2: // jnz
        if (!ZF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xc3:
        jmp;
        break;
    case 0xc4:
        if (!ZF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xc5:
        push(get_bc());
        break;
    case 0xc6:
        add(memory::read_memory_byte(PC++));
        break;
    case 0xc7:
        rst(0x00);
        break;
    case 0xc8:
        if (ZF)
            ret;
        break;
    case 0xc9:
        ret; // ret
        break;
    case 0xca:
        if (ZF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xcb:
        jmp;
        break;
    case 0xcc:
        if (ZF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xcd:
        call;
        break;
    case 0xce:
        adc(memory::read_memory_byte(PC++));
        break;
    case 0xcf:
        rst(0x01);
        break;
    case 0xd0:
        if (!CF)
            ret;
        break;
    case 0xd1:
        set_de(pop());
        break;
    case 0xd2: // jnc
        if (!CF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xd3:
        port_out();
        break;
    case 0xd4:
        if (!CF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xd5:
        push(get_de());
        break;
    case 0xd6:
        sub(memory::read_memory_byte(PC++));
        break;
    case 0xd7:
        rst(0x10);
        break;
    case 0xd8:
        if (CF)
            ret;
        break;
    case 0xd9:
        ret;
        break;
    case 0xda:
        if (CF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xdb:
        port_in();
        break;
    case 0xdc:
        if (CF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xdd:
        call;
        break;
    case 0xde:
        sbb(memory::read_memory_byte(PC++));
        break;
    case 0xdf:
        rst(0x18);
        break;
    case 0xe0:
        if (!PF)
            ret;
        break;
    case 0xe1:
        set_hl(pop());
        break;
    case 0xe2:
        if (!PF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xe3: // xthl
        data16 = memory::read_memory_word(SP);
        memory::write_memory_word(SP, get_hl());
        set_hl(data16);
        break;
    case 0xe4: // cpo
        if (!PF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xe5:
        push(get_hl());
        break;
    case 0xe6:
        ana(memory::read_memory_byte(PC++));
        break;
    case 0xe7:
        rst(0x20);
        break;
    case 0xe8:
        if (PF)
            ret;
        break;
    case 0xe9:
        PC = get_hl(); // pchl
        break;
    case 0xea: // jpe
        if (PF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xeb:
        data16 = get_de(); // xchg
        set_de(get_hl());
        set_hl(data16);
        break;
    case 0xec:
        if (PF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xed:
        call;
        break;
    case 0xee:
        xra(memory::read_memory_byte(PC++));
        break;
    case 0xef:
        rst(0x28);
        break;
    case 0xf0:
        if (!SF)
            ret;
        break;
    case 0xf1: // pop psw
        AF = pop();
        A = AF >> 8;
        PSW = AF & 0xff;
        SF = (!SF) ? PSW >> 7 & 0x1 : !PSW >> 7 & 0x1;
        ZF = (!ZF) ? PSW >> 6 & 0x1 : !PSW >> 6 & 0x1;
        AC = (!AC) ? PSW >> 4 & 0x1 : !PSW >> 4 & 0x1;
        PF = (!PF) ? PSW >> 2 & 0x1 : !PSW >> 2 & 0x1;
        CF = (!CF) ? PSW >> 0 & 0x1 : !PSW >> 0 & 0x1;
        break;
    case 0xf2:
        if (!SF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xf3:
        break;
    case 0xf4:
        if (!SF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xf5: // push psw
        PSW |= SF << 0x7;
        PSW |= ZF << 0x6;
        PSW |= AC << 0x4;
        PSW |= PF << 0x2;
        PSW |= CF << 0x0;
        push(A << 8 | PSW);
        break;
    case 0xf6:
        ora(memory::read_memory_byte(PC++));
        break;
    case 0xf7:
        rst(0x30);
        break;
    case 0xf8: // rm
        if (SF)
            ret;
        break;
    case 0xf9:
        SP = get_hl();
        break;
    case 0xfa:
        if (SF)
        {
            jmp;
        }
        else
            PC += 2;
        break;
    case 0xfb:
        break; // not implemented
    case 0xfc:
        if (SF)
        {
            call;
        }
        else
            PC += 2;
        break;
    case 0xfe:
        cmp(memory::read_memory_byte(PC++));
        break;
    case 0xff:
        rst(0x38);
        break;
    case 0xfd:
        call;
        break;
    default:
        std::cout << "*** Instruction not implemented " << memory::memory[PC] << std::endl;
    }
}

byte_t i8080::get_register_c()
{
    return C;
}

word_t i8080::get_pc()
{
    return PC;
}

word_t i8080::get_register_hl()
{
    return get_hl();
}

word_t i8080::get_register_bc()
{
    return get_bc();
}

byte_t i8080::get_register_e()
{
    return E;
}

word_t i8080::get_register_de()
{
    return get_de();
}

byte_t i8080::get_register_d()
{
    return D;
}

byte_t i8080::get_register_b()
{
    return B;
}

byte_t i8080::get_register_l()
{
    return L;
}

int i8080::get_cycles()
{
    return cycles;
}

byte_t *i8080::memory_addr()
{
    return memory::memory;
}

void i8080::load_file_bin(std::string name, byte_t *load, word_t jump)
{
    std::fstream file(name, std::ios::binary);
    int size = 0;
    PC = jump;

    memset(memory::memory, 0, 0x10000);

    file.open(name);
    if (file.is_open())
    {
        while (!file.eof())
        {
            file.read((char *)load + jump, 1028);
            size += file.gcount();
            load += size;
            if (size > MAX_MEMORY)
            {
                std::cout << "*** memory not supported, memory intel this suport 65536c " << std::endl;
                exit(1);
            }
        }
        std::cout << "*** file " << name << " loaded size " << size << ", memory jumped to " << PC << std::endl
                  << std::endl;
    }
    else
    {
        std::cout << "*** file " << name << " not found or not permission for read" << std::endl;
        exit(2);
    }
}

void i8080::i8080_init()
{
    flags_init();
}

void i8080::i8080_instructions()
{
    execute_opcode(memory::read_memory_byte(PC++));
}