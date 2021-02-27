(defconst wabi-version "0.0.1")

(defgroup wabi nil
  "Wabi group"
  :group 'languages)

(defvar wabi-mode-syntax-table
  (let((table (make-syntax-table lisp-mode-syntax-table)))
    table))
(defconst wabi-sym-forbidden-rest-chars
  "][\";\\^`~\(\)\{\}\\,\s\t\n\r")

(defconst wabi-sym-forbidden-1st-chars
  (concat wabi-sym-forbidden-rest-chars "0-9:'"))

(defconst wabi-sym-regexp
  (concat "[^" wabi-sym-forbidden-1st-chars "][^" wabi-sym-forbidden-rest-chars "]*"))

(defconst wabi-column-atom-regexp
  ":[^];{}()#]*")

(defconst wabi-atom-regexp
  "\\(:[^];{}()#]*\\|()\\|_\\)")

(defconst wabi-number-regexp
  "\\([0-9][0-9_]*\\)")

(defconst wabi-core-combiners
  (list
   ;; builtins
   "def" "if" "do" "pr" "eval" "clock" "not" "hash" "l0" "collect" "load"
   ;; combiners
   "fx" "wrap" "unwrap" "comb" "fx\\?" "fn\\?" "cont\\?"
   ;; list
   "cons" "car" "cdr" "pair\\?" "list\\?" "len"
   ;; numbers
   "+" "*" "-" "/"
   ;; binarites
   "bin\\?" "bin-len" "bin-cat" "bin-sub"
   ;; compare
   "=" "/=" ">" "<" ">=" "<="
   ;; env
   "env\\?" "ext"
   ;; continuations
   "prompt" "control" "prmt" "ctrl"
   ;; map
   "map/new" "assoc" "dissoc" "map-len" "map\\?" ;"map-get"
   ;; symbols
   "sym\\?" "sym"
   ;; places
   "plc" "plc\\?" "plc-val" "plc-cas"
   ;; vectors
   "vec" "vec-len" "vec\\?" "push-right" "push-left" "right" "left" "pop-left" "pop-right" "vec-concat" "vec-set"
   ;; l1
   ;; "q" "qs" "id" "list" "cmt" "env" "apply" "list*"
   ;; "fn" "defx" "defn"
   ;; "prmt" "ctrl"
   ;; "fold" "foldr" "conc" "part" "flip" "rev" "always" "all\\?" "some\\?"
   ;; "last" "but-last"
   ;; "->" "->>"
   ;; "or" "and"
   ;; "map" "each"
   ;; "rec" "let" "letr"
   ;; "comp" "inc" "dec" "zero\\?" "one\\?" "pos\\?" "neg\\?" "ign\\?"
   ;; "take" "drop" "filter" "remove" "find" "in\\?" "take\\?" "drop\\?" "iter" "ran"
   ;; "snd" "trd" "fth" "nth"
   ;; "when" "unless" "awhen" "afn"
   ;; "vec-update" "plc-swap" "type"
   ))

(defconst wabi-ws-regexp
  "[ \r\n\t]+")

(defconst wabi-symbol-boundary-regexp
  "[ \";\\^`~\(\)\{\}\\,\s\t\n\r]")

(defconst wabi-builtin-regexp
  (concat wabi-symbol-boundary-regexp
          "\\(" (regexp-opt wabi-core-combiners t) "\\)"
          wabi-symbol-boundary-regexp))

(defconst wabi-comment-regexp
  "\\(;;.*\n\\)")

(defun wabi-add-keywords (mode)
  (font-lock-remove-keywords mode '("(\\(do\\)[ \r\n\t()]" 1 font-lock-keyword-face))
  (font-lock-add-keywords
   mode
   `(
     (,wabi-comment-regexp
      (1 font-lock-comment-face))

     (,wabi-number-regexp
      (1 font-lock-constant-face))

     ("(\\(def\\)[ \r\n\t]+(\\([/[:alpha:]*!_-][^(){}[:space:]]*\\))[ \r\n\t]"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face))

     ("(\\(def\\)[ \r\n\t]+\\([/[:alpha:]*!_-][^(){}[:space:]]*\\)[ \r\n\t]"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face))

     ("(\\(def\\)[ \r\n\t]*(\\(.*\\))"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face))

     ("(\\(defx\\)[ \r\n\t]+\\([/[:alpha:]*!_-][^(){}[:space:]]*\\)[ \r\n\t]"
      (1 font-lock-keyword-face)
      (2 font-lock-function-name-face))

     ("(\\(defn\\)[ \r\n\t]+\\([/[:alpha:]*!_-][^(){}[:space:]]*\\)[ \r\n\t]"
      (1 font-lock-keyword-face)
      (2 font-lock-function-name-face))

     ("(\\(prompt\\)[ \r\n\t]+\\([/[:alpha:]*!_-][^(){}[:space:]]*\\)[ \r\n\t]"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face))

     ("(\\(control\\)[ \r\n\t]+\\([/[:alpha:]*!_-][^(){}[:space:]]*\\)[ \r\n\t]+\\([/[:alpha:]*!_-][^(){}[:space:]]*\\)[ \r\n\t]"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face)
      (3 font-lock-function-name-face))


     ("(\\(rec\\)[ \r\n\t]+\\([/[:alpha:]*!_-][^(){}[:space:]]*\\)[ \r\n\t]"
      (1 font-lock-keyword-face)
      (2 font-lock-function-name-face))

     (,wabi-builtin-regexp
      (1 font-lock-builtin-face))

     (,wabi-atom-regexp
      (1 font-lock-constant-face))

     )))

(defun wabi-indent ()
  (put 'let 'lisp-indent-function 'defun)
  (put 'letr 'lisp-indent-function 'defun)
  (put 'rec 'lisp-indent-function 2)
  (put 'def 'lisp-indent-function 1)
  (put 'defn 'lisp-indent-function 'defun)
  (put 'defx 'lisp-indent-function 3)
  (put 'defg 'lisp-indent-function 1)
  (put 'defm 'lisp-indent-function 3)
  (put 'shift 'lisp-indent-function 1)
  (put 'when 'lisp-indent-function 1)
  (put 'unless 'lisp-indent-function 1)
  (put 'when-let 'lisp-indent-function 2)
  (put 'fn 'lisp-indent-function 1)
  (put 'fx 'lisp-indent-function 2)
  (put 'eval 'lisp-indent-function 1)
  (put 'if 'lisp-indent-function 2)
  (put 'defpackage 'lisp-indent-function 3)
  (put 'defpackages 'lisp-indent-function 3)
  (put 'defstruct 'lisp-indent-function 2)
  (put 'defstructs 'lisp-indent-function 2)
  (put 'definterface 'lisp-indent-function 1)
  (put 'do lisp-indent-function 1)
  (put 'control lisp-indent-function 2)
  (put 'prompt lisp-indent-function 1)
  (put 'ctrl lisp-indent-function 2)
  (put 'prmt lisp-indent-function 1))

(define-derived-mode wabi-mode lisp-mode "Wabi mode"
  "A major mode to edit wabi files"
  :group 'wabi
  :syntax-table wabi-mode-syntax-table
  (wabi-indent)
  (wabi-add-keywords 'wabi-mode))

(add-to-list 'auto-mode-alist '("\\.wabi\\'" . wabi-mode))

(provide 'wabi-mode)
