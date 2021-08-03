# Format

`deepen` stores its information in regular files and folders in the filesystem.

* Store in file when leaf
* Store in folder/index when non-leaf

## Filename

* Leaf: `\d\d-<name>.md`
* Non-leaf:`\d\d-<name>/index.md`
* This ensures that a file/folder can easily be moved

* How to hiden items that are done

## Id

* Start with `#`
* Optional `?` indicates the numeric part of the #id should be generated
* Format: `#<namespace><number>`

## Markdown

Deepen supports [markdown](https://daringfireball.net/projects/markdown/) files with the `.md` file extension. On top of the regular `markdown` formatting, it adds a few additional directives to gradually restructure and rework the stored information.

### Title

* Depth of Title can only increase with unit step
* Rework Title depth to make it consistent and correct
* Optional #id in title