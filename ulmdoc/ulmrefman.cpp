#include <filesystem>
#include <fstream>
#include <sstream>

#include "expr.hpp"
#include "exprwrapper.hpp"
#include "hdw.hpp"
#include "instrfmt.hpp"
#include "ulmdoc.hpp"

using namespace ulmdoc;

#include <ulm1/_gen_refman_format.cpp>

int
main(int argc, const char **argv)
{
    if (argc != 4) {
	fprintf(stderr, "Example usage: %s isa.txt template.tex out.tex\n",
		argv[0]);
	return 1;
    }

    std::ifstream tex, isa;
    std::string isaTxt = argv[1];
    std::string mainTex = argv[2];
    std::string refmanTex = argv[3];

    tex.open(mainTex);
    if (!tex.is_open()) {
	std::cerr << argv[0] << ": can not open input file '" << mainTex << "'"
		  << std::endl;
    }

    isa.open(isaTxt);
    if (!isa.is_open()) {
	std::cerr << argv[0] << ": can not open input file '" << isaTxt << "'"
		  << std::endl;
    }

    std::ofstream out;
    out.open(refmanTex);
    if (!out.is_open()) {
	std::cerr << argv[0] << ": can not open output file '" << refmanTex
		  << "'" << std::endl;
    }

#include <ulm1/_gen_refman_instr.cpp>

    ulmDoc.print(tex, isa, out);

    tex.close();
    out.close();
}
