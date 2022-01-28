# deepen

Utilities, format and process to gradually deepen your understanding of a project

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