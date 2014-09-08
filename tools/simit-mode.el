;;; simit-mode.el --- 

;;; Commentary:
; Installation: Place simit-mode.el in a folder in the emacs path
; (e.g. ~/.emacs.d/), Then add '(load "simit-mode")' to your emacs
; init file (e.g. ~/.emacs).

;;; Code:

;; command to comment/uncomment text
(defun simit-comment-dwim (arg)
 "Comment or uncomment current line or region in a smart way.
For detail, see `comment-dwim'."
 (interactive "*P")
 (require 'newcomment)
 (let (
       (comment-start "%") (comment-end "")
       )
   (comment-dwim arg)))

;; keywords for syntax coloring
(setq simit-keywords
      `(
        ( ,(regexp-opt '("") 'word)
        . font-lock-function-name-face)
        ( ,(regexp-opt
        '("const" "extern" "proc" "func" "map" "to" "with" "reduce" "while" "if"
          "elif" "else" "end" "return" "struct")
        'words) . font-lock-keyword-face)
        ( ,(regexp-opt '("") 'words) . font-lock-constant-face)
        ( ,(regexp-opt '() 'words) . font-lock-builtin-face)
        ( ,(regexp-opt
        '("int" "float" "Tensor")
        'words) . font-lock-type-face) ) )

;; syntax table
(defvar simit-syntax-table nil "Syntax table for `simit-mode'.")
(setq simit-syntax-table
      (let ((synTable (make-syntax-table)))

        ;; Simit comments (only single line support at the moment)
        (modify-syntax-entry ?% "<" synTable)
        (modify-syntax-entry ?\n ">"  synTable)

        synTable))

;; define the major mode.
(define-derived-mode simit-mode fundamental-mode
  "simit-mode is a major mode for editing language simit."
  :syntax-table simit-syntax-table
  
  (setq font-lock-defaults '(simit-keywords))
  (setq mode-name "simit")

  (setq tab-width 4)

  ; Indent with true tabs
  (local-set-key [?\C-t] 'tab-to-tab-stop)
  (local-set-key [?\t] 'tab-to-tab-stop)

  ; Indent with spaces
  (setq indent-tabs-mode nil)

  ; Save buffer as pdf
  (local-set-key [?\C-x ?\g] 'simit-pdf-save)

  ;; modify the keymap
  (define-key simit-mode-map [remap comment-dwim] 'simit-comment-dwim)
)

(add-to-list 'auto-mode-alist '("\\.sim\\'" . simit-mode))

(provide 'simit-mode)

