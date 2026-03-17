#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class SyntaxHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  SyntaxHighlighter(const QString &language, QTextDocument *parent = nullptr);
  void setLanguage(const QString &lang);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct Rule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QList<Rule> m_rules;
  void buildRules(const QString &language);
};
