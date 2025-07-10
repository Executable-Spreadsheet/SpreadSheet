# Rendering Functions


```c
void drawBox(v2u pos, v2u size, SString str);
```
The draw box function takes in a position, size and string.
The box is drawn with the top left corner at pos, and the border
has dimensions size.

This means that there is size - (2,2) area inside the border.
The string must be clamped outside otherwise it will spill.

```c
SString cellDisplay(Allocator mem, SpreadSheet* sheet, v2u pos, u32 maxlen);
```

This function takes in a sheet, position, and a maximum length to return
a sized string containing the 'first' maxlen characters of the data.

It will format the output as well using standard library snprintf. The
returned string is allocated using the provided allocator.

> As a side note, I (Eli) recommend using a stack with this function
since it can be super performant and can be cleared and reset in constant
time which allows for it to be used in a loop without issue.
