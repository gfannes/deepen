# Create WBS @C:61.6% @E:4h10m @S:r @T:1h36m

* From single file

## Metadata @C:100% @E:1h35m @S:V @T:0m

* `metadata/Item.hpp`
    * Create `metadata::Item` as key-value of `std::string`
        * There should be a difference between, eg
            * `@abc:`: key="abc", value=""
            * `@abc`:  key="",    value="abc"
        * Parse and serialize @20m @V
    * Create `split()` to split line in `text` and `metadata::Items`
        * On first occurence of `@` character @10m @V
* Store durations in minutes
    * Parse and serialize in `metadata/Duration.hpp` @30m @V
        * Support for failing parse
    * Store duration minutes as double
    * Do not output fractions of minutes
* Make `Section.metadata` optional @5m @V
    * Necessary to implement the Minimal State Aggregate
* Classify `metadata` items into the different types
    * Create Status class
        * State enum
            * Check with `proast` states: has additional `starting` state @V
        * `done` state
        * Parse and serialize @15m @V
        * `fraction_done()` @15m @V
    * 3 types of metadata, in this order
        * Unrecognised Items
        * Input metadata
            * Effort: `@2w2d2h2m`
            * Status: `@r`, `@R`, `@d`, `@D`, `@s`, `@S`, `@i`, `@I`, `@v`, `@V`
                * Requirement, Design, Starting, Implementation and Validation
                * Each take 20% of the effort
                * Capital indicate completion state: with capital: done
        * Aggregated metadata
            * Todo: time left
            * Minimal state: `@S:Imp` all implementation is done
            * Completion: `@c:60%`

## Section @C:100% @E:15m @S:V @T:0m

* Support for output to file @15m @V

## CLI @C:71.4% @E:35m @S:r @T:10m

* Add `-u|--update` option @5m @I
    * Output loaded Sections to file, with metadata
    * Default output file is input file @5m
* Add `-e|--export` option @5m @I
    * Output loaded Sections to file, without metadata
    * Do not overwrite the input file
* Aggregate metadata on loaded Sections
    * Rename `Aggdata` into `Aggrdata` @5m @V
    * Aggregate metadata @15m @v

## Parsing Markdown @C:63.3% @E:30m @S:r @T:11m

* Create Section Type enum: Title, Line @5m @V
    * Stream support @5m @V
* Nest Lines in Title Section directly during parsing @5m @I
    * Only support for Line for now
    * Update UT @10m
* Remove `Section.lines` @5m @V

## Neovim @C:0% @E:1h15m @S:r @T:1h15m

* Add key binding to add effort @30m
* Add key binding to set/change state @15m
* Add key binding to update using `deepen` @30m
