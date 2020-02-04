# TV Show Model

This directory contains data containers that are used by the `TvShowModel`.
Note that none of the files ending in `*Item.cpp` are actually models according
to the QT model/view concept. They are solely data containers and do not
provide any functionality for signaling the addition or removal of seasons,
episodes, etc.

If you intend to add seasons or episodes, go through `TvShowModel`.
Special care needs to be taken when adding or removing rows from the model.
That is, `beginInsertRows` and similar need to be called whenever rows are
added or removed! None of the item classes in this directory so that.
