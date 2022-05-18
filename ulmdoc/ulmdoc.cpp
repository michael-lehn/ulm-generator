#include <iostream>

#include "ulmdoc.hpp"

static std::string
replace(std::string str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
	str.replace(start_pos, from.length(), to);
	start_pos += to.length();
    }
    return str;
}

//------------------------------------------------------------------------------

namespace ulmdoc {

UlmDoc ulmDoc;

void
UlmDoc::setActiveKey(const Key &key)
{
    activeKey = key;
}

void
UlmDoc::addDescription(const Description &description)
{
    page[activeKey] += description;
    page[activeKey] += "\n";
}

void
UlmDoc::addMnemonicDescription(const Key &key, const Description &description)
{
    intro[key] += description;
    intro[key] += "\n";
}

void
UlmDoc::addUnescapedDescription(const Description &description)
{
    std::string escaped = replace(description, "\\", "\\\\");
    escaped = replace(description, "%", "\\%");
    escaped = replace(escaped, "$", "\\$");
    page[activeKey] += escaped;
    page[activeKey] += "\n";
}

void
UlmDoc::addLineBreak()
{
    page[activeKey] += "\\\\[0.3cm]\n";
}

void
UlmDoc::addFormat()
{
    page[activeKey] += "\\subsubsection*{Format}\n";
}

void
UlmDoc::addPurpose()
{
    page[activeKey] += "\\subsubsection*{Purpose}\n";
}

void
UlmDoc::addEffect()
{
    page[activeKey] += "\\subsubsection*{Effect}\n";
}

void
UlmDoc::addAsmNotation()
{
    page[activeKey] += "\\subsection{Assembly Notation}\n";
}

void
UlmDoc::addAsmAlternative()
{
    page[activeKey] += "\\subsubsection*{Alternative Assembly Notation}\n";
}

/*
    std::cout << "\\documentclass{article}" << std::endl;
    std::cout << "\\usepackage{amsmath}" << std::endl;
    std::cout << "\\usepackage{tikz}" << std::endl;
    std::cout << "\\usepackage{titlesec}" << std::endl;
    std::cout << "\\titlespacing*{\\subsection}{0pt}{9.5ex}{4.3ex}"
	      << std::endl;
    std::cout << "\\begin{document}" << std::endl;

    // ...

    std::cout << "\\end{document}" << std::endl;
*/

void
UlmDoc::print(std::ifstream &tex, std::ofstream &out) const
{
    for (std::string line; std::getline(tex, line);) {
	if (line == "%ISA") {
	    for (const auto &p : page) {
		out << "\\newpage" << std::endl;
		out << "\\section{" << p.first << "}" << std::endl;
		if (intro.count(p.first)) {
		    out << intro.find(p.first)->second << std::endl;
		}
		out << p.second << std::endl;
	    }
	} else {
	    out << line << std::endl;
	}
    }

    tex.close();
}

} // namespace ulmdoc
