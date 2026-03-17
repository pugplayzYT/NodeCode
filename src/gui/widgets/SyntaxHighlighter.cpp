#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(const QString &language, QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
  buildRules(language);
}

void SyntaxHighlighter::setLanguage(const QString &lang) {
  buildRules(lang);
  rehighlight();
}

void SyntaxHighlighter::buildRules(const QString &language) {
  m_rules.clear();

  QTextCharFormat keywordFmt;
  keywordFmt.setForeground(QColor(86, 156, 214));  // blue
  keywordFmt.setFontWeight(QFont::Bold);

  QTextCharFormat stringFmt;
  stringFmt.setForeground(QColor(206, 145, 120));  // orange

  QTextCharFormat commentFmt;
  commentFmt.setForeground(QColor(106, 153, 85));  // green
  commentFmt.setFontItalic(true);

  QTextCharFormat numberFmt;
  numberFmt.setForeground(QColor(181, 206, 168));  // light green

  QTextCharFormat typeFmt;
  typeFmt.setForeground(QColor(78, 201, 176));  // teal

  QTextCharFormat funcFmt;
  funcFmt.setForeground(QColor(220, 220, 170));  // yellow-ish

  if (language == "C++") {
    QStringList keywords = {"auto", "break", "case", "class", "const", "continue",
      "default", "delete", "do", "else", "enum", "extern", "false", "for",
      "if", "inline", "int", "long", "namespace", "new", "nullptr", "operator",
      "private", "protected", "public", "return", "short", "signed", "sizeof",
      "static", "struct", "switch", "template", "this", "throw", "true", "try",
      "typedef", "unsigned", "using", "virtual", "void", "volatile", "while",
      "bool", "char", "double", "float", "string", "include"};
    for (const auto &kw : keywords)
      m_rules.append({QRegularExpression("\\b" + kw + "\\b"), keywordFmt});

    QStringList types = {"std::string", "std::vector", "std::map", "std::cout",
      "std::cin", "std::endl", "std::thread", "std::mutex"};
    for (const auto &t : types)
      m_rules.append({QRegularExpression(QRegularExpression::escape(t)), typeFmt});

  } else if (language == "Python") {
    QStringList keywords = {"and", "as", "assert", "async", "await", "break",
      "class", "continue", "def", "del", "elif", "else", "except", "False",
      "finally", "for", "from", "global", "if", "import", "in", "is",
      "lambda", "None", "nonlocal", "not", "or", "pass", "raise", "return",
      "True", "try", "while", "with", "yield"};
    for (const auto &kw : keywords)
      m_rules.append({QRegularExpression("\\b" + kw + "\\b"), keywordFmt});

  } else if (language == "JavaScript") {
    QStringList keywords = {"async", "await", "break", "case", "catch", "class",
      "const", "continue", "debugger", "default", "delete", "do", "else",
      "export", "extends", "false", "finally", "for", "function", "if",
      "import", "in", "instanceof", "let", "new", "null", "of", "return",
      "super", "switch", "this", "throw", "true", "try", "typeof",
      "undefined", "var", "void", "while", "with", "yield"};
    for (const auto &kw : keywords)
      m_rules.append({QRegularExpression("\\b" + kw + "\\b"), keywordFmt});
  }

  // Common rules for all languages
  m_rules.append({QRegularExpression("\\b[0-9]+(\\.[0-9]+)?\\b"), numberFmt});
  m_rules.append({QRegularExpression("\\b\\w+(?=\\()"), funcFmt});
  m_rules.append({QRegularExpression("\"[^\"]*\""), stringFmt});
  m_rules.append({QRegularExpression("'[^']*'"), stringFmt});

  if (language == "Python") {
    m_rules.append({QRegularExpression("#[^\n]*"), commentFmt});
  } else {
    m_rules.append({QRegularExpression("//[^\n]*"), commentFmt});
  }
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
  for (const auto &rule : m_rules) {
    auto it = rule.pattern.globalMatch(text);
    while (it.hasNext()) {
      auto match = it.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}
