(defconst wabi-version "0.0.1")

(defgroup wabi nil
  "Wabi group"
  :group 'languages)

(defvar wabi-mode-syntax-table
  (let((table (make-syntax-table emacs-lisp-mode-syntax-table)))
    table))

(defun wabi-list-of
    (n)
  (concat "(\\(" n +space+ "\\)*"))

(defun wabi-add-keywords (mode)
  (font-lock-remove-keywords mode '("(\\(do\\)[([:space:]]" 1 font-lock-keyword-face))
  (font-lock-add-keywords
   mode
   `(
     ("(\\(def\\)[[:space:]]+\\([/[:alpha:]*!][^(){}[:space:]]*\\)[[:space:]]+"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face))

     ("(\\(def\\)[[:space:]]*(\\(.*\\))"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face))

     ("(\\(defx\\)[[:space:]]+\\([/[:alpha:]*!][^(){}[:space:]]*\\)[[:space:]]+"
      (1 font-lock-keyword-face)
      (2 font-lock-function-name-face))

     ("(\\(defn\\)[[:space:]]+\\([/[:alpha:]*!][^(){}[:space:]]*\\)[[:space:]]+"
      (1 font-lock-keyword-face)
      (2 font-lock-function-name-face))

     ("(\\(prompt\\)[[:space:]]+\\([/[:alpha:]*!][^(){}[:space:]]*\\)[[:space:]]+"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face))

     ("(\\(control\\)[[:space:]]+\\([/[:alpha:]*!][^(){}[:space:]]*\\)[[:space:]]+\\([/[:alpha:]*!][^(){}[:space:]]*\\)[[:space:]]+"
      (1 font-lock-keyword-face)
      (2 font-lock-variable-name-face)
      (3 font-lock-function-name-face))

     ("(\\(fn\\)[\[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(fx\\)[\[([:space:]]" 1 font-lock-keyword-face)

     ("(\\(rec\\)[[:space:]]+\\([/[:alpha:]*!][^(){}[:space:]]*\\)[[:space:]]"
      (1 font-lock-keyword-face)
      (2 font-lock-function-name-face))

     ("(\\(let\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(when\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(when-not\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(if\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(cond\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(reset\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(eval\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(/>\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(and\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(or\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(set!\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(do\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(wrap\\)[([:space:]]" 1 font-lock-keyword-face)
     ("(\\(unwrap\\)[([:space:]]" 1 font-lock-keyword-face)

     ("(\\(each\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(map\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(apply\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(fold\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(flip\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(ran\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(comp\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(always\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(car\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cdr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cadr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cddr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(caddr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cdddr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cadddr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cddddr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(last\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(but-last\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(all\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(some\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(conc\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(filter\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(remove\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(in?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(evens\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(odds\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(len\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(inc\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(dec\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(drop\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(take\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(push!\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(pop!\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cons\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(car\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(cdr\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(nil\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(pair\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(sym\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(env\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(wrap\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(fexp\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(err\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(builtin\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(num\\?\\)[([:space:]]" 1 font-lock-builtin-face)
     ("(\\(halt\\)[([:space:]]" 1 font-lock-builtin-face)
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
  (put 'prompt lisp-indent-function 1))

(define-derived-mode wabi-mode lisp-mode "Wabi mode"
  "A major mode to edit wabi files"
  :group 'wabi
  :syntax-table wabi-mode-syntax-table
  (wabi-indent)
  (wabi-add-keywords 'wabi-mode))

(add-to-list 'auto-mode-alist '("\\.wabi\\'" . wabi-mode))

(provide 'wabi-mode)
