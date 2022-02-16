# deepen

Utilities, format and process to gradually deepen your understanding of a project

## Installation

* Install [gubg](https://github.com/gfannes/gubg)
```
git clone https://github.com/gfannes/gubg
cd gubg
git submodule update --init --recursive
export gubg=`pwd`
rake uth
rake prepare
export RUBYLIB=$gubg/ruby
export PATH=$PATH:$gubg/bin
cd ..
```
* Install deepen
```
git clone https://github.com/gfannes/deepen
cd deepen
ln -s $gubg gubg
rake install
```

# List of emoticons

https://github.com/ikatyang/emoji-cheat-sheet

# Emoji with pandoc-latext

```
---
header-includes: |
    \usepackage{emoji}
...

<span style="color:blue">some *This is Blue italic.* text</span>

Roses are \textcolor{red}{text in red}, violets are \textcolor{blue}{text in blue}.

\emoji{joy}
\emoji{snail}
```

Compile with `pandoc -f markdown+emoji --pdf-engine=lualatex test.md -o test.pdf`

Not sure how the `:joy:` and `:snail:` can be put to work

# Colored text with pandoc-latex

`\textcolor{color}{text}`