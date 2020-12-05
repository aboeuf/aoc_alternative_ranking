#pragma once

#include <QString>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>

namespace json_helper
{

template <typename T>
struct Type
{
  static QJsonValue::Type get() { return QJsonValue::Undefined; }
  static T cast(const QJsonValue&) { return T{}; }
};

template <>
struct Type<bool>
{
  static QJsonValue::Type get() { return QJsonValue::Bool; }
  static bool cast(const QJsonValue& value) { return value.toBool(); }
};

template <>
struct Type<int>
{
  static QJsonValue::Type get() { return QJsonValue::Double; }
  static int cast(const QJsonValue& value) { return value.toInt(); }
};

template <>
struct Type<double>
{
  static QJsonValue::Type get() { return QJsonValue::Double; }
  static double cast(const QJsonValue& value) { return value.toDouble(); }
};

template <>
struct Type<QString>
{
  static QJsonValue::Type get() { return QJsonValue::String; }
  static QString cast(const QJsonValue& value) { return value.toString(); }
};

template <>
struct Type<QJsonArray>
{
  static QJsonValue::Type get() { return QJsonValue::Array; }
  static QJsonArray cast(const QJsonValue& value) { return value.toArray(); }
};

template <>
struct Type<QJsonObject>
{
  static QJsonValue::Type get() { return QJsonValue::Object; }
  static QJsonObject cast(const QJsonValue& value) { return value.toObject(); }
};

} // namespace json_helper

class JsonHelper
{
public:
  QString getTypeName(const QJsonValue::Type& type)
  {
    switch (type) {
      case QJsonValue::Null:
        return "null";
      case QJsonValue::Bool:
        return "bool";
      case QJsonValue::Double:
        return "double";
      case QJsonValue::String:
        return "string";
      case QJsonValue::Array:
        return "array";
      case QJsonValue::Object:
        return "object";
      default:
        return "undefined";
    }
  }

  bool read(const QString& filepath, QJsonDocument& document)
  {
    QFile file(filepath);
    if (!file.open(QFile::Text | QFile::ReadOnly)) {
      m_error = "Cannot open file \"" + QFileInfo(file).absoluteFilePath() + "\"";
      return false;
    }
    QJsonParseError parse_error;
    document = QJsonDocument::fromJson(file.readAll(), &parse_error);
    if (document.isNull()) {
      m_error = "Parsing error in file \"" + QFileInfo(file).absoluteFilePath() + "\":\n" +
                parse_error.errorString();
      return false;
    }
    if (document.isEmpty()) {
      m_error = "File \"" + QFileInfo(file).absoluteFilePath() + "\" does not contain any data";
      return false;
    }
    if (!document.isArray() && !document.isObject()) {
      m_error = "File \"" + QFileInfo(file).absoluteFilePath() +
                "\" does not contain a valid JSON array nor object";
      return false;
    }
    return true;
  }

  bool read(const QString& filepath, QJsonArray& array)
  {
    QJsonDocument document;
    if (!read(filepath, document))
      return false;
    if (!document.isArray()) {
      m_error = "File \"" + QFileInfo(QFile(filepath)).absoluteFilePath() +
                "\" contains a valid JSON object but a JSON array was expected";
      return false;
    }
    array = document.array();
    return true;
  }

  bool read(const QString& filepath, QJsonObject& object)
  {
    QJsonDocument document;
    if (!read(filepath, document))
      return false;
    if (!document.isObject()) {
      m_error = "File \"" + QFileInfo(QFile(filepath)).absoluteFilePath() +
                "\" contains a valid JSON array but a JSON object was expected";
      return false;
    }
    object = document.object();
    return true;
  }

  template <typename T>
  bool read(const QJsonValue& json, T& value)
  {
    const auto value_type = json_helper::Type<T>::get();
    const auto json_type = json.type();
    if (value_type != json_type) {
      m_error = "Cannot cast read value of type " + getTypeName(json_type) +
                " into expected type " + getTypeName(value_type);
      return false;
    }
    if (value_type == QJsonValue::Undefined) {
      m_error = "Undefined template value type";
      return false;
    }
    value = json_helper::Type<T>::cast(json);
    return true;
  }

  template <typename T>
  bool read(const QJsonObject& object, const QString& key, T& value)
  {
    const auto json_value = object[key];
    if (json_value == QJsonValue::Undefined) {
      m_error = "Input JSON object does not contain the key \"" + key + "\"";
      return false;
    }
    return read(json_value, value);
  }

  template <typename T, typename Integer>
  bool read(const QJsonArray& array, Integer index, T& value)
  {
    const auto i = static_cast<int>(index);
    if (i < 0 or i >= array.size()) {
      m_error = QString("Out of range index %1 (casted to %2)").arg(index).arg(i);
      return false;
    }
    return read(array[i], value);
  }

  const QString& error() { return m_error; }

private:
  QString m_error = "";
};
