# deepen

Utilities, format and process to gradually deepen your understanding of a project.

Deepen provides the CLI tool `dpn` that can be used to:
* Create a hierarchical discription of your project, split over different files
* Use [markdown](https://daringfireball.net/projects/markdown/) and [textile](https://textile-lang.com/) and even the combination of both
* Add metadata to titles, subtitles and bullets
  * Effort, status information, tags
* Aggregate this metadata over the hierachy
* Update the files in-place or export to a single file, eg. for further processing with [pandoc](https://pandoc.org/)
* List the title and subtitles that are not completed yet

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

## Usage

* `dpn U <input file>`: recompute all metadata aggregates starting from `<input file>`
* `dpn u <input file>`: remove all derived metadata aggregates
* `dpn e -i <input file> -o <output file>`: recompute all metadata aggregates and export to a single file
* `dpn l -i <input file>`: list incompleted title tasks to find the next task that should be executed
* `dpn r <command>`: run `<command>` in each of the root folders found in the configuration file

## Configuration

`dpn` tries to load a configuration file from `$HOME/.config/dpn.naft` using the following syntax:
```
[root](fp:/home/geertf/gubg)(name:gubg)
[root](fp:/home/geertf/deepen)(name:dpn)
```
These root folders can be iterated with the `dpn r` command, and the name for each root can be used to create links between different documents using eg. `${gubg}` to refer to the "/home/geertf/gubg" folder.

## Metadata

Each title, subtitle and bullet can be provided with metadata. All manually provided metadata start with the `@` character:
* Effort indication is weeks, days, hours and minutes, eg. `@1d2h` to indicate a job that will take 1 day (of 8 hours) + 2 hours.
* Status indication. `dpn` knows 5 different levels of completion, use lower case to indicate it is still in-progress, and upper case to indicate completion. Each completed level counts for 20% of the total duration:
  * `@r`, `@R`: Requirements collection
  * `@d`, `@D`: Designing the solution
  * `@s`, `@S`: Starting development
  * `@i`, `@I`: Implementing the solution
  * `@v`, `@V`: Validation
  * A task can be indicated as _cancelled_ via appending the `~`, eg, `@i~`.

A link to another document is created using the `@[text](filepath)` syntax, where `filepath` can be a relative or absolute path to the document to be included. To link across different projects, you can use the `${name}` syntax to refer to roots from the configuration.


## Random information

### List of emoticons

https://github.com/ikatyang/emoji-cheat-sheet

### Emoji with pandoc-latext

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

### Colored text with pandoc-latex

`\textcolor{color}{text}`