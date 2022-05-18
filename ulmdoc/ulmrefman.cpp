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
    if (argc != 3) {
	fprintf(stderr, "Example usage: %s template.tex out.tex\n", argv[0]);
	return 1;
    }

    std::ifstream tex;
    std::string mainTex = argv[1];
    std::string refmanTex = argv[2];

    tex.open(mainTex);
    if (!tex.is_open()) {
	std::cerr << argv[0] << ": can not open input file '" << mainTex << "'"
		  << std::endl;
    }

    std::ofstream out;
    out.open(refmanTex);
    if (!out.is_open()) {
	std::cerr << argv[0] << ": can not open output file '" << refmanTex
		  << "'" << std::endl;
    }

#include <ulm1/_gen_refman_instr.cpp>

    ulmDoc.print(tex, out);

    tex.close();
    out.close();
}
