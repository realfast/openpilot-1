#include <QPushButton>
#include <QDoubleValidator>

#include "input.hpp"
#include "qt_window.hpp"

InputDialog::InputDialog(const QString &prompt_text, QWidget *parent) : QDialog(parent) {
  layout = new QVBoxLayout();
  layout->setContentsMargins(50, 50, 50, 50);
  layout->setSpacing(20);

  // build header
  QHBoxLayout *header_layout = new QHBoxLayout();

  label = new QLabel(prompt_text, this);
  label->setStyleSheet(R"(font-size: 75px; font-weight: 500;)");
  header_layout->addWidget(label, 1, Qt::AlignLeft);

  QPushButton* cancel_btn = new QPushButton("Cancel");
  cancel_btn->setStyleSheet(R"(
    padding: 30px;
    padding-right: 45px;
    padding-left: 45px;
    border-radius: 7px;
    font-size: 45px;
    background-color: #444444;
  )");
  header_layout->addWidget(cancel_btn, 0, Qt::AlignRight);
  QObject::connect(cancel_btn, SIGNAL(released()), this, SLOT(reject()));
  QObject::connect(cancel_btn, SIGNAL(released()), this, SIGNAL(cancel()));

  layout->addLayout(header_layout);

  // text box
  layout->addSpacing(20);
  line = new QLineEdit();
  line->setStyleSheet(R"(
    border: none;
    background-color: #444444;
    font-size: 80px;
    font-weight: 500;
    padding: 10px;
  )");
  layout->addWidget(line, 1, Qt::AlignTop);

  k = new Keyboard(this);
  QObject::connect(k, SIGNAL(emitButton(const QString&)), this, SLOT(handleInput(const QString&)));
  layout->addWidget(k, 2, Qt::AlignBottom);

  setStyleSheet(R"(
    * {
      color: white;
      background-color: black;
    }
  )");

  setLayout(layout);
}

QString InputDialog::getText(const QString &prompt, int minLength) {
  InputDialog d = InputDialog(prompt);
  d.setMinLength(minLength);
  const int ret = d.exec();
  return ret ? d.text() : QString();
}

QString InputDialog::getConfigDecimal(const QString &prompt, std::string existingValue, float min, float max) {
  InputDialog d = InputDialog(prompt);
  const QString existing = QString::fromStdString(existingValue);
  d.setText(existing);
  d.setMinLength(1);
  const auto validator = new QDoubleValidator(min, max, 4);
  d.line->setValidator(validator);
  d.k->setKeyboardLayout(2); // numbers
  const int ret = d.exec();
  if (ret) {
    auto input = d.text();
    int pos = 0;
    if (validator->validate(input, pos) == QValidator::Acceptable) {
      return d.text();
    }
  }
  return QString();
}

QString InputDialog::text() {
  return line->text();
}

int InputDialog::exec() {
  setMainWindow(this);
  return QDialog::exec();
}

void InputDialog::show(){
  setMainWindow(this);
}

void InputDialog::handleInput(const QString &s) {
  if (!QString::compare(s,"⌫")) {
    line->backspace();
  }

  if (!QString::compare(s,"⏎")) {
    if (line->text().length() >= minLength){
      done(QDialog::Accepted);
      emitText(line->text());
    } else {
      setMessage("Need at least "+QString::number(minLength)+" characters!", false);
    }
  }

  QVector<QString> control_buttons {"⇧", "↑", "ABC", "⏎", "#+=", "⌫", "123"};
  for(QString c : control_buttons) {
    if (!QString::compare(s, c)) {
      return;
    }
  }

  line->insert(s.left(1));
}

void InputDialog::setMessage(const QString &message, bool clearInputField){
  label->setText(message);
  if (clearInputField){
    line->setText("");
  }
}

void InputDialog::setMinLength(int length){
  minLength = length;
}

void InputDialog::setText(const QString &text) {
  line->setText(text);
}

ConfirmationDialog::ConfirmationDialog(const QString &prompt_text, const QString &confirm_text, const QString &cancel_text,
                                       QWidget *parent):QDialog(parent) {
  setWindowFlags(Qt::Popup);
  layout = new QVBoxLayout();
  layout->setMargin(25);

  prompt = new QLabel(prompt_text, this);
  prompt->setWordWrap(true);
  prompt->setAlignment(Qt::AlignHCenter);
  prompt->setStyleSheet(R"(font-size: 55px; font-weight: 400;)");
  layout->addWidget(prompt, 1, Qt::AlignTop | Qt::AlignHCenter);

  // cancel + confirm buttons
  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->setSpacing(20);
  btn_layout->addStretch(1);
  layout->addLayout(btn_layout);

  if (cancel_text.length()) {
    QPushButton* cancel_btn = new QPushButton(cancel_text);
    btn_layout->addWidget(cancel_btn, 0, Qt::AlignRight);
    QObject::connect(cancel_btn, SIGNAL(released()), this, SLOT(reject()));
  }

  if (confirm_text.length()) {
    QPushButton* confirm_btn = new QPushButton(confirm_text);
    btn_layout->addWidget(confirm_btn, 0, Qt::AlignRight);
    QObject::connect(confirm_btn, SIGNAL(released()), this, SLOT(accept()));
  }

  setFixedSize(900, 350);
  setStyleSheet(R"(
    * {
      color: black;
      background-color: white;
    }
    QPushButton {
      font-size: 40px;
      padding: 30px;
      padding-right: 45px;
      padding-left: 45px;
      border-radius: 7px;
      background-color: #44444400;
    }
  )");

  setLayout(layout);
}

bool ConfirmationDialog::alert(const QString &prompt_text) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, "Ok", "");
  return d.exec();
}

bool ConfirmationDialog::confirm(const QString &prompt_text) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text);
  return d.exec();
}

int ConfirmationDialog::exec() {
   // TODO: make this work without fullscreen
#ifdef QCOM2
  setMainWindow(this);
#endif
  return QDialog::exec();
}
