rem address of the array
let arr: 64
let len: 64
let i: 0

rem filling the array with numbers
for:
  poke i * 2, arr + i
  let i: i + 1
  if i < len: for, $

rem now you have an array of numbers from 0 to 126 with step 2

rem printing the array
for2:
  peek holder, arr + i
  print $i$: $holder$\n
  let i: i + 1
  if i < len: for2, $