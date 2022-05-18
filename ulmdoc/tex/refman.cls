% vim: syntax=tex
%------------------------------------------------------------------------------
% Class for reference manuals. Following options are supported:
%
%    a4paper        select ISO A4 format
%    twoside        common for books
%
%    graaf          side layout after Graaf
%    bringhurst     specialized ISO A4 side layout after Bringhurst
%
%    garamond       Garamond family         \
%    times          Times Roman              \   always along with
%    newcent        New Century Schoolbook   /   matching math fonts
%    palatino       Palatino                /
%
%    english        English text
%    german         German text
%------------------------------------------------------------------------------
\NeedsTeXFormat{LaTeX2e}
\ProvidesPackage{refman}[2009/12/17 LaTeX class for reference manuals]
% geometry options
\DeclareOption{a4paper}{%
   \PassOptionsToPackage{a4paper}{geometry}%
   \PassOptionsToClass{a4paper}{article}%
}
\DeclareOption{twoside}{%
   \PassOptionsToPackage{twoside}{geometry}%
   \PassOptionsToClass{twoside}{article}%
}
\DeclareOption{graaf}{%
   \PassOptionsToPackage{
      marginratio=1:2,
      hscale=0.6666666,
      vscale=0.6666666
   }{geometry}%
}
\DeclareOption{bringhurst}{%
   \PassOptionsToPackage{
      hdivide={0.111111\paperwidth,*,0.222222\paperwidth},
      vdivide={0.111111\paperwidth,*,0.222222\paperwidth},
   }{geometry}%
}
% font family
\DeclareOption{garamond}{%
   \renewcommand{\familydefault}{ugm}%
   \AtEndOfClass{\RequirePackage[garamond]{mathdesign}}%
}
\DeclareOption{times}{%
   \AtEndOfClass{%
      \RequirePackage{times}%
      \RequirePackage{mathtime}%
   }%
}
\DeclareOption{newcent}{%
   \AtEndOfClass{%
      \RequirePackage{newcent}%
      \RequirePackage{fouriernc}%
      % an alternative option may be millennial
      % as soon as it becomes available
      % check here: http://www.math.uiuc.edu/~hartke/computer/latex/latex.php
   }%
}
\DeclareOption{palatino}{%
   \AtEndOfClass{%
      \RequirePackage{palatino}%
      % alternatives are pxfonts or mathpazo
      \RequirePackage{mathpple}%
   }%
}
% language settings
\DeclareOption{german}{%
   \PassOptionsToPackage{german}{babel}%
}
\DeclareOption{english}{%
   \PassOptionsToPackage{english}{babel}
}
% hyperref settings
% ={#1}
%\DeclareOptionX{pageanchor}{%
%   \PassOptionsToPackage{\CurrentOption={#1}}{hyperref}
%}
%\DeclareOptionX{bookmarks}{%
%   \PassOptionsToPackage{\CurrentOption={#1}}{hyperref}
%}
%\DeclareOptionX{bookmarksnumbered}{%
%   \PassOptionsToPackage{\CurrentOption={#1}}{hyperref}
%}
%\DeclareOptionX{bookmarksopen}{%
%   \PassOptionsToPackage{\CurrentOption={#1}}{hyperref}
%}
%\DeclareOptionX{plainpages}{%
%   \PassOptionsToPackage{\CurrentOption={#1}}{hyperref}
%}
%\DeclareOption{pdfpagelabels}{%
%   \PassOptionsToPackage{\CurrentOption}{hyperref}
%}
%% pass all other options to the book class
%\DeclareOption*{%
%   \PassOptionsToClass{\CurrentOption}{book}%
%}
\ProcessOptions\relax
% load base class and packages
\LoadClass{report}
\RequirePackage{geometry}
\RequirePackage[T1]{fontenc}
\RequirePackage{textcomp}
\RequirePackage{babel}
\RequirePackage[utf8]{inputenc}
\RequirePackage{graphicx}
\RequirePackage{float}
\RequirePackage{afterpage}
%\RequirePackage{amsmath}
\RequirePackage{amssymb}
\RequirePackage{latexsym}
\RequirePackage{moreverb}
\RequirePackage{color}
\RequirePackage{pifont}
\RequirePackage{makeidx}
\RequirePackage{ifthen}
\RequirePackage[
   pageanchor=false,
   bookmarks=true,
   bookmarksnumbered=false,
   bookmarksopen=false,
   plainpages=false,
   pdfpagelabels
]{hyperref}
\RequirePackage{thumbpdf}
\RequirePackage{listings}
\RequirePackage{fancyhdr}
\RequirePackage{tabularx}
\RequirePackage{longtable}
\RequirePackage{natbib}
\AtEndOfClass{%
   \RequirePackage[protrusion=true,expansion=false]{microtype}
   \DeclareGraphicsExtensions{.pdf,.png,.jpg,.mps}
}
\pdfcompresslevel=9
\frenchspacing

% recommendations of Donald Arsenau for floats,
% see <9806021119.ZM7656@guave>#1/1 in comp.text.tex
\renewcommand{\topfraction}{.85}
\renewcommand{\bottomfraction}{.7}
\renewcommand{\textfraction}{.15}
\renewcommand{\floatpagefraction}{.66}
\renewcommand{\dbltopfraction}{.66}
\renewcommand{\dblfloatpagefraction}{.66}
\setcounter{topnumber}{9}
\setcounter{bottomnumber}{9}
\setcounter{totalnumber}{20}
\setcounter{dbltopnumber}{9}

% taken from http://www-solar.mcs.st-andrews.ac.uk/~robertc/latex/Latex.html
\makeatletter
\def\cleardoublepage{\clearpage\if@twoside \ifodd\c@page\else%
\hbox{}%
\thispagestyle{empty}
\newpage%
\if@twocolumn\hbox{}\newpage\fi\fi\fi}
\makeatother

%------------------------------------------------------------------------------
% headers
%------------------------------------------------------------------------------
\pagestyle{fancyplain}
% \addtolength{\headwidth}{\marginparsep}
% \addtolength{\headwidth}{\marginparwidth}
\renewcommand{\chaptermark}[1]{\markboth{#1}{}}
\renewcommand{\sectionmark}[1]{\markright{\thesection\ #1}}
\lhead[\fancyplain{}{\bfseries\thepage}]%
   {\fancyplain{}{\nouppercase{\bfseries\rightmark}}}
\rhead[\fancyplain{}{\nouppercase{\bfseries\leftmark}}]%
   {\fancyplain{}{\bfseries\thepage}}
\cfoot{}

%------------------------------------------------------------------------------
% natbib configuration
%------------------------------------------------------------------------------
\let\cite\citep
\bibliographystyle{plainnat}
\bibpunct{[}{]}{,}{a}{}{;}

%------------------------------------------------------------------------------
% tabularx configuration
%------------------------------------------------------------------------------
% taken from LaTeX Companion, 2nd ed, p. 249, 251
\newcolumntype{Y}{>{\raggedright\arraybackslash}X}

%------------------------------------------------------------------------------
% typographical correct \n, \t ... (don't use $\backslash$ outside math)
% Application: \bs{n}, \bs{t}, ...
%------------------------------------------------------------------------------
\newcommand{\bs}[1]{{\char'134{}#1}}
\newcommand{\bsbs}{\char'134\char'134}
\newcommand{\tildeSY}{\char'176}
\newcommand{\barSY}{\char'174}
\newcommand{\caretSY}{\char'136}
\newcommand{\backslashSY}{\char'134}
\newcommand{\leftbracketSY}{\char'133}
\newcommand{\rightbracketSY}{\char'135}

%------------------------------------------------------------------------------
% environment for grammars
%------------------------------------------------------------------------------
\newlength{\grammarwidth}%
\setlength{\grammarwidth}{\textwidth}%
\addtolength{\grammarwidth}{-2\fboxsep}%
\newenvironment{grammar}{%
   \smallskip%
   \noindent%
   \color{syntax}%
   \begin{eqnarray*}
}{%
   \end{eqnarray*}
   \smallskip%
}
\newcommand{\produces}{& \longrightarrow &~}
\newcommand{\nonterminal}[1]{%
   \relax%
   \ifmmode\langle\mbox{#1}\index{#1}\rangle~%
   %\else$\langle\mbox{#1}\index{#1}\rangle$%
   \else$\langle${#1}\index{#1}$\rangle$%
   \fi%
}
\newcommand{\lextoken}[1]{\mbox{``{#1}''\index{#1}}~}
\newcommand{\token}[1]{``{#1}''\index{#1}}
\newcommand{\lexkeyword}[1]{\mbox{\keyword{#1}}~}
\newcommand{\optional}[1]{\left[~{#1}\right]~}
\newcommand{\multiple}[1]{\left\{~{#1}\right\}~}
\newcommand{\nextline}{\\ & &~}
\newcommand{\alternative}{\mid~}
%------------------------------------------------------------------------------
% let everything be black, please
% (originally there was a distinction between screen and printer output
% which is not preserved)
%------------------------------------------------------------------------------
\definecolor{keyword}{rgb}{0,0,0}%
\definecolor{ident}{rgb}{0,0,0}%
\definecolor{comment}{rgb}{0,0,0}%
\definecolor{string}{rgb}{0,0,0}%
\definecolor{session}{rgb}{.9,.9,.9}%
\definecolor{syntax}{rgb}{0,0,0}%
\definecolor{emph}{rgb}{0,0,0}%
\definecolor{emphbf}{rgb}{0,0,0}%
\definecolor{cite}{rgb}{0,0,0}%
\definecolor{citecol}{rgb}{0,0,0}%
\definecolor{pagecol}{rgb}{0,0,0}%
\definecolor{urlcol}{rgb}{0,0,0}%

%------------------------------------------------------------------------------
% the following stuff comes from the old macros.tex file
%------------------------------------------------------------------------------
%% Gliederungsnummerierung bis Tiefe ... (3 => bis subsubsection!)
\setcounter{secnumdepth}{3}
%% Inhaltsverzeichnis soll Gliederung bis ... enthalten
\setcounter{tocdepth}{3}

%%
%% macht die Nummer des Kapitels verfuegbar, sofern
%% die Kapitel auch immer mit \inputkapitel eingebunden werden
%% (=> erleichtert die Angabe von Dateinamen, da die Pfad
%%     automatisch ergaenzt werden koennen)
%%
\newcommand{\kapitel}{1}
\newcommand{\kapinput}[1]{
   \renewcommand{\kapitel}{#1}%
   \input{#1/kap}%
}

%%
%% referenziert ein Objekt aus dem akt. Kapitel
%% (opt. wird der Typ, also fig oder prog, der Referenz angegeben)
%%
\newcommand{\kapref}[2][prog]{%
   \ref{#1:\kapitel:#2}%
}
\newcommand{\kappageref}[2][prog]{%
   \pageref{#1:\kapitel:#2}%
}

%%
%% referenziert ein Objekt aus einem anderen Kapitel
%% (opt. wird der Typ, also fig oder prog, der Referenz angegeben)
%%
\newcommand{\otherkapref}[3][prog]{%
   \ref{#1:#2:#3}%
}

%%
%% einbinden einer Grafik
%% (wird im pics/ Verzeichnis des akt. Kapitels gesucht)
%% Der Suffix des Dateinamens wird hier festgelegt,
%% damit die Quellen unabhaengig davon bleiben, ob wir pdflatex
%% oder latex verwenden
%%
\newcommand{\Grafik}[3][]{%
   \begin{figure}[htbp!]%
      \begin{center}%
	 \ifthenelse{\equal{#1}{}}{%
	    \includegraphics{\kapitel/pics/#2.pdf}%
	 }{%
	    \includegraphics[width=#1]{\kapitel/pics/#2.pdf}%
	 }
	 \caption{#3}%
	 \label{fig:\kapitel:#2}%
      \end{center}%
   \end{figure}%
}         

%%
%% Einstellungen zum listings Paket
%%
%\usepackage[breaklines]{listings}
\lstdefinelanguage{astl}{
   morekeywords={abstract, and, as, at, attribution, cache, close, create, 
      cut, delete, div, else, elsif, exists, foreach, if, import, in, left, 
      library, machine, mod, nonassoc, null, on, operators, opset, or, post, 
      pre, print, private, retract, return, right, rules, shared, state, 
      sub, transformation, var, when, where, while, x},
   sensitive=true,
   morecomment=[l]{//},
   morecomment=[s]{/*}{*/},
   morestring=[b]"
}
\lstloadlanguages{astl,C}

%\lstset{labelsep=10pt,labelstep=1,indent=25pt,captionpos=t}
\lstset{breaklines=true,numbersep=10pt,stepnumber=1,captionpos=t}
\lstset{basewidth={0.60em,0.48em},columns=[l]fullflexible,flexiblecolumns=true}
\lstset{frame=,escapechar=`,extendedchars=true}
\lstset{basicstyle={},%\ttfamily,
   keywordstyle=\bfseries\color{keyword},
   identifierstyle=\itshape\color{ident},
   commentstyle=\itshape\color{comment},
   stringstyle=\color{string}}
\lstset{language=astl}

\renewcommand{\lstlistlistingname}{Sample Programs}
\renewcommand{\lstlistingname}{Program}

% macros that should conform to the listings configuration
\newcommand{\keyword}[1]{\textbf{#1}\index{#1}}
%\newcommand{\ident}[1]{{\itshape\color{ident}#1\/}}
\newcommand{\ident}[1]{{\itshape{}#1\/}}

%%
%% ein Programm einfuegen
%% (wird im progs/ Verzeichnis des akt. Kapitels gesucht)
%%
%\floatstyle{ruled}
%\newfloat{Program}{thp}{lop}[chapter]
\newcommand{\Programm}[3][]{%
   \smallskip%
   \noindent%
   \ifthenelse{\equal{#1}{}}{%
       \lstinputlisting[frame=tb,label=prog:\kapitel:#2,caption={[#3]#3 (\emph{#2})}]{\kapitel/progs/#2}%
   }{%
       \lstinputlisting[frame=tb,label=prog:\kapitel:#2,caption={[#3]#3 (\emph{#2})},#1]{\kapitel/progs/#2}%
   }%
   \smallskip%
}

%%
%% eine Session einfuegen
%% (wird im progs/ Verzeichnis des akt. Kapitels gesucht)
%%
\newlength{\sessionwidth}%
\setlength{\sessionwidth}{\textwidth}%
\addtolength{\sessionwidth}{-2\fboxsep}%
\newcommand{\Session}[1]{%
   \smallskip%
   \noindent%
   \colorbox{session}{%
      \parbox{\sessionwidth}{%
	 \noindent%
	 \verbatimtabinput[8]{\kapitel/progs/#1}%
      }%
   }%
   \smallskip%
}

%%
%% Kapitel ohne Nummerierung, aber mit einem
%% Eintrag in das Inhaltsverzeichnis
%%
\newcommand{\chapterno}[1]{%
   \chapter*{#1}%
        \addcontentsline{toc}{chapter}{#1}%
   \markboth{#1}{#1}%
   \setcounter{footnote}{0}%
}

%%
%% Redefinition von \emph, so dass die Farbe des
%% hervorgehobenen Textes angepasst werden kann
%%
\let\oldemph=\emph
\renewcommand{\emph}[1]{\protect\textcolor{emph}{\oldemph{#1}}}

%%
%% Neudefinition von \emphbf statt \textbf, so dass die Farbe des
%% hervorgehobenen Textes angepasst werden kann
%%
\newcommand{\emphbf}[1]{\protect\textcolor{emphbf}{\textbf{#1}}}

%%
%% Redefinition von \cite, so dass die Farbe des
%% Zitates angepasst werden kann
%%
\let\oldcite=\cite
\renewcommand{\cite}[1]{\protect\textcolor{cite}{\oldcite{#1}}}
