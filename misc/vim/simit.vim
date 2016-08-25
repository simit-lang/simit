" Vim syntax file
" Language: Simit	
" Last Change: 14 July 2016	
"
" Add the following lines to your ~/.vimrc to highlight .sim files:
" syntax on
" au BufNewFile,BufRead *.sim set filetype=simit 

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn keyword simitType         float int bool vector matrix tensor string set inout
syn keyword simitStatement    element func proc export
syn keyword simitConditional  if elif else end
syn keyword simitRepeat       for in while do
syn keyword simitStorageClass const extern var
syn keyword simitFunc         apply map to reduce print println

syn keyword simitBuiltins      mod sin cos tan asin acos atan2 sqrt log exp pow  
syn keyword simitBuiltins      clock storeTime
syn keyword simitBuiltins      norm dot det inv lu lufree lusolve lumatsolve chol cholfree lltsolve lltmatsolve
syn keyword simitBuiltins      createComplex createNorm complexGetReal complexGetImag complexConj

syn keyword simitTodo contained TODO NOTE FIXME XXX

syn match  simitRelationalOperator "\(==\|\!=\|>=\|<=\|=\~\|>\|<\|=\)"
syn match simitArithmeticOperator  "[-+]"
syn match simitArithmeticOperator  "\.\=[*/\\^]"
syn match simitRangeOperator       ":"
syn keyword simitLogicalOperator   and not or
syn keyword simitBoolean           true false

syn match simitNumber		"\<\d\+[ij]\=\>"
syn match simitFloat		"\<\d\+\(\.\d*\)\=\([edED][-+]\=\d\+\)\=[ij]\=\>"
syn match simitFloat		"\.\d\+\([edED][-+]\=\d\+\)\=[ij]\=\>"
syn keyword simitConstant	pi


"syn match simitDelimiter        "[][()]"
syn match simitTransposeOperator "[])a-zA-Z0-9.]'"lc=1

syn match simitSemicolon          ";"

syn match simitComment            "%.*$" contains=simitTodo,simitTab
syn region simitBlockComment      start=+%{+    end=+%}+ contains=simitBlockComment

syn region simitString start=+\v"+ skip=+\v\\.+ end=+\v"+

" linear algebra
syn keyword simitFunc     norm cross dot trace eye      
" trig
syn keyword simitFunc     sin cos tan asin acos atan2
" exponential
syn keyword simitFunc     exp log pow sqrt
" rounding and remainder
syn keyword simitFunc     ceil floor mod 
" filters
syn keyword simitFunc filter

if version >= 508 || !exists("did_simit_syntax_inits")
  if version < 508
    let did_simit_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink simitType Type

  HiLink simitTransposeOperator  simitOperator
  HiLink simitConditional        Conditional
  HiLink simitRepeat             Repeat
  HiLink simitTodo               Todo
  HiLink simitDelimiter          Identifier
  HiLink simitNumber             Number
  HiLink simitFloat              Float
  HiLink simitConstant           Constant
  HiLink simitStatement          Statement
  HiLink simitSemicolon          SpecialChar
  HiLink simitComment            Comment
  HiLink simitBlockComment       Comment
  HiLink simitString             String
  HiLink simitBoolean            Boolean
  HiLink simitStorageClass       StorageClass

  HiLink simitArithmeticOperator simitOperator
  HiLink simitRangeOperator      simitOperator
  HiLink simitRelationalOperator simitOperator
  HiLink simitLogicalOperator    simitOperator
  HiLink simitOperator           Operator
  HiLink simitFunc               Function
  HiLink simitBuiltins           Function

  delcommand HiLink
endif

let b:current_syntax = "simit"
