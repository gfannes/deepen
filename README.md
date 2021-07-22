# deepen

Utilities, format and process to gradually deepen your understanding of a project

## Filesystem format &format/

`deepen` stores its information in regular files and folders in the filesystem. It currently supports [markdown](https://daringfireball.net/projects/markdown/) files with the `.md` file extension. On top of the regular `markdown` formatting, it adds a few additional directives to gradually restructure and rework the stored information.

### Title

The `Title` of a `Section` is the first line that starts with `#+ `.

#### Identifiers

Identifiers start with the `#` symbol, and are used to identify `Stories`.

#### Folders

Folders start with the `&` symbol.

## Operations

### Split file into files and folders

---
dpn: hello
