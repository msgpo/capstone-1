//===-- RISCVInstPrinter.cpp - Convert RISCV MCInst to asm syntax ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an RISCV MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifdef CAPSTONE_HAS_RISCV

#include <stdio.h>		// DEBUG
#include <stdlib.h>
#include <string.h>
#include <capstone/platform.h>

#include "RISCVInstPrinter.h"
#include "RISCVBaseInfo.h"
#include "../../MCInst.h"
#include "../../SStream.h"
#include "../../MCRegisterInfo.h"
#include "../../utils.h"
#include "RISCVMapping.h"

//#include "RISCVDisassembler.h"

#define GET_REGINFO_ENUM
#include "RISCVGenRegisterInfo.inc"
#define GET_INSTRINFO_ENUM
#include "RISCVGenInstrInfo.inc"

static void printRegName(SStream * OS, unsigned RegNo);
static void printOperand(MCInst * MI, unsigned OpNo, SStream * O);
static void printFenceArg(MCInst * MI, unsigned OpNo, SStream * O);

// Autogenerated by tblgen.
static void printInstruction(MCInst * MI, SStream * O, MCRegisterInfo * MRI);
static char *printAliasInstr(MCInst * MI, SStream * OS, void *info);
static void printCustomAliasOperand(MCInst * MI, unsigned OpIdx,
				    unsigned PrintMethodIdx, SStream * OS);
//Originally had default value unsigned AltIdx = RISCV_ABIRegAltName. Now we are passing this value whenever this function is called.
static const char *getRegisterName(unsigned RegNo, unsigned AltIdx);

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "RISCVGenAsmWriter.inc"

void RISCV_post_printer(csh ud, cs_insn * insn, char *insn_asm, MCInst * mci)
{
	/*
	   if (((cs_struct *)ud)->detail != CS_OPT_ON)
	   return;
	 */
}

//void RISCVInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
//                                 StringRef Annot, const MCSubtargetInfo &STI) 
void RISCV_printInst(MCInst * MI, SStream * O, void *info)
{
	MCRegisterInfo *MRI = (MCRegisterInfo *) info;

	if (!printAliasInstr(MI, O, info))
		printInstruction(MI, O, MRI);
	//printAnnotation(O, Annot);
}

static void printRegName(SStream * OS, unsigned RegNo)
{
	SStream_concat0(OS, getRegisterName(RegNo, RISCV_ABIRegAltName));
}

//void RISCVInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
//                                  raw_ostream &O, const char *Modifier) 
static void printOperand(MCInst * MI, unsigned OpNo, SStream * O)
{
	unsigned reg;
	int64_t Imm = 0;

	if (OpNo >= MI->size)
		return;
	MCOperand *MO = MCInst_getOperand(MI, OpNo);

	if (MCOperand_isReg(MO)) {
		reg = MCOperand_getReg(MO);
		printRegName(O, reg);
		//TODO_rod: Other archs check also for "if (MI->csh->doing_mem)". Tricore is not. Verify if it is needed. 
		//Sparc and tricore use this function "reg = Sparc_map_register(reg);" from SparcMapping.c - Check if it is needed or if the reg obtained before matches.
		if (MI->csh->detail) {
			MI->flat_insn->detail->riscv.operands[MI->flat_insn->detail->riscv.op_count].type = RISCV_OP_REG;
			MI->flat_insn->detail->riscv.operands[MI->flat_insn->detail->riscv.op_count].reg = reg;
			MI->flat_insn->detail->riscv.op_count++;
		}

		return;
	}

	if (MCOperand_isImm(MO)) {
		Imm = MCOperand_getImm(MO);
		if (Imm >= 0) {
			if (Imm > HEX_THRESHOLD)
				SStream_concat(O, "0x%" PRIx64, Imm);
			else
				SStream_concat(O, "%" PRIu64, Imm);
		} else {
			if (Imm < -HEX_THRESHOLD)
				SStream_concat(O, "-0x%" PRIx64, -Imm);
			else
				SStream_concat(O, "-%" PRIu64, -Imm);
		}

		if (MI->csh->detail) {
			MI->flat_insn->detail->riscv.operands[MI->flat_insn->detail->riscv.op_count].type = RISCV_OP_IMM;
			MI->flat_insn->detail->riscv.operands[MI->flat_insn->detail->riscv.op_count].imm = Imm;
			MI->flat_insn->detail->riscv.op_count++;
		}
		return;
	}
	//MO.getExpr()->print(O, &MAI);
}

static void printFenceArg(MCInst * MI, unsigned OpNo, SStream * O)
{
	MCOperand *MO = MCInst_getOperand(MI, OpNo);
	unsigned FenceArg = MCOperand_getImm(MO);

	if ((FenceArg & RISCVFenceField_I) != 0)
		SStream_concat0(O, "i");
	if ((FenceArg & RISCVFenceField_O) != 0)
		SStream_concat0(O, "o");
	if ((FenceArg & RISCVFenceField_R) != 0)
		SStream_concat0(O, "r");
	if ((FenceArg & RISCVFenceField_W) != 0)
		SStream_concat0(O, "w");
}

//TODO_rod: So far I am not including this functions unless it is really needed.
/*void RISCVInstPrinter::printFRMArg(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  auto FRMArg =
      static_cast<RISCVFPRndMode::RoundingMode>(MI->getOperand(OpNo).getImm());
  O << RISCVFPRndMode::roundingModeToString(FRMArg);
}*/

#endif				// CAPSTONE_HAS_RISCV
