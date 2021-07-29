# CLI application

## Input

* From a single file
* From several file/folder trees
* Support for loading different file formats: `.md`, `.jira`, `.naft`, `.todo`

## Output

* To a single file or file/folder tree
* Different file formats: `.md`, `.html`, `.jira`

## Operations

### Split

* Split files at a given level. It is important that all childs from a file are split at the same level, otherwise, we need support for interleaving external and internal nodes.
* Remove input files that are not part of the output anymore to make sure duplicate info is not present
  * Only when we are in overwrite mode

### Rename

### Search/list

* For #id and @tag

### Sort

* By importance or priority

### Update

* Metadata
* File/folder tree: export

### Run

* Command in root folders
  * Can be used to pull/commit these repos

### Generate

* Ids

## Config

* Store all config info in `.config/dpn/config.naft`
* Support for different profiles
  * Root files to use
* List config profiles