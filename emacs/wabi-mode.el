(defconst wabi-version "0.0.1")

(defgroup wabi nil
  "Wabi group"
  :group 'languages)

(defvar wabi-mode-syntax-table
  (let((table (make-syntax-table lisp-mode-syntax-table)))
    table))

(defun wabi-add-keywords (mode)
  (font-lock-remove-keywords mode '("(\\(do\\)[ \r\n\t()]" 1 font-lock-keyword-face))
  (font-lock-add-keywords
   mode
   `(
     ("\\(;.*\n\\)"
      (1 font-lock-comment-face))

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

     ("(\\(plc\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(let\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(prmt\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(ctrl\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(when\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(unless\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(if\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(eval\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(->\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(and\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(or\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(do\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(fn\\)[\[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(fx\\)[\[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(wrap\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(unwrap\\)[ \r\n\t()]" 1 font-lock-keyword-face)
     ("(\\(each\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(map\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(apply\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(fold\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(flip\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(ran\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(comp\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(part\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(iter\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(always\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(cons\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(car\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(cdr\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(hd\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(tl\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(fst\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(snd\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(trd\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(fth\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(nth\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(last\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(but-last\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(all\\?\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(some\\?\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(conc\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(filter\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(remove\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(in\\?\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(evens\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(find\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(odds\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(len\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(inc\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(dec\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(drop\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(take\\)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(nil\\?\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(pair\\?\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(sym\\?\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     ("(\\(env\\^)[ \r\n\t()]]" 1 font-lock-builtin-face)
     ("(\\(num\\?\\)[ \r\n\t()]" 1 font-lock-builtin-face)
     )))

(defun wabi-indent ()
  (put 'let 'lisp-indent-function 'defun)
  (put 'rec 'lisp-indent-function 2)
  (put 'def 'lisp-indent-function 1)
  (put 'defn 'lisp-indent-function 'defun)
  (put 'defx 'lisp-indent-function 3)
  (put 'shift 'lisp-indent-function 1)
  (put 'when 'lisp-indent-function 1)
  (put 'when-not 'lisp-indent-function 1)
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
