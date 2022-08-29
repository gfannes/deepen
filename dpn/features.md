# Features

## Input formats
### .. Markdown
* .@ Support line number
### .. Naft
* .@ Support line number
### .@ Freemind
* .@ Implement gubg/xml/Reader
### .? Source code
* .@ Support line number
* Line-based commend identification

## GTD lists
* Provide inspection via TUI
** Toggle list
** 2-pane: list + node view

## Metadata
* .@ Aggregate costs at different levels
* .. !B Change &include: to &include()
* .@ Aggregate metadata through Nodes and Files

## .? Priority
* The priority number should be computed after the cost has been aggregated
* The priority number should not be affected by the level of detail: compute it where specified and aggregate with max

## Configuration
* .. Load default inputs from config file
* .@ !a Use gubg/naft/Reader to load Config
* .? Support specifying an editor for the different file formats
** With line number

## Options
* .? Support specifying the verb after the options

## Sublime
### Commands
* .? Support for setting/altering the state, cost, duedate, prio
* .? Support for navigating includes
### Syntax highlighting
* .@ Set color depending on state
* .@ Set highlighter as link

## QC
* .@ Add UTs
* .@ Add QCs
** Use `naft --pack` to handle files and folders