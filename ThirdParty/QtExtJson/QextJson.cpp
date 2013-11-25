#include "QextJson.h"
#include <QVariant>
#include <QDebug>
#include <QTextStream>
#include <QStringList>

QString QextJson::stringify(QVariant v){
    if (v.isNull()){
        return "null";
    }// QXT dev's: WTF????? mb just return empty string? with best regards, flareguner.
    int t = v.type();
    if (t == QVariant::String){
        return "\""+v.toString().replace("\\", "\\\\").replace("\"", "\\\"")+"\"";
    }
    else if (t == QVariant::Bool){
        return v.toBool()?"true":"false";
    }else if (t ==  QVariant::Int){
        return QString::number(v.toInt());
    }else if (t ==  QVariant::Double){
        return QString::number(v.toDouble());
    }else if (t == QVariant::Map){
        QString r="{";
        QMap<QString, QVariant> map = v.toMap();
        QMapIterator<QString, QVariant> i(map);
        while (i.hasNext()){
            i.next();
            r+="\""+i.key()+"\":"+stringify(i.value())+",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="}";
        return r;
    }else if (t == QVariant::Hash){
        QString r="{";
        QHash<QString, QVariant> map = v.toHash();
        QHashIterator<QString, QVariant> i(map);
        while (i.hasNext()){
            i.next();
            r+="\""+i.key()+"\":"+stringify(i.value())+",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="}";
        return r;
    }else if (t == QVariant::StringList){
        QString r="[";
        QStringList l = v.toStringList();
        foreach(const QString& i, l){
            r+="\""+i+"\",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="]";
        return r;
    }else if (t == QVariant::List){
        QString r="[";
        QVariantList l = v.toList();
        foreach(const QVariant& i, l){
            r+=stringify(i)+",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="]";
        return r;
    }

    return QString();
}

static QVariant parseValue(QTextStream &s,bool & error);
static QVariantMap parseObject (QTextStream & s,bool & error);
static QVariantList parseArray (QTextStream & s,bool & error);
static QString parseString (QTextStream & s,bool & error);
static QVariant parseLiteral (QTextStream & s,bool & error);

QVariant QextJson::parse(QString string){
    QTextStream s(&string);
    bool error=false;
    QVariant v=parseValue(s,error);
    if(error)
        return QVariant();
    return v;
}



static QVariant parseValue(QTextStream &s,bool & error){
    s.skipWhiteSpace();
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c=='{'){
            return parseObject(s,error);
        } else if (c=='"'){
            return parseString(s,error);
        } else if (c=='['){
            return parseArray(s,error);
        } else {
            return parseLiteral(s,error);
        }
        s.skipWhiteSpace();
    }
    return QVariant();
}

static QVariantMap parseObject (QTextStream & s,bool & error){
    s.skipWhiteSpace();
    QVariantMap o;
    QString key;
    bool atVal=false;

    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c=='}'){
            return o;
        } else if (c==',' || c==':'){
            /*
              They're syntactic sugar, since key:value come in bundles anyway
              Could check for error handling. too lazy.
            */
        } else if (c=='"'){
            if(atVal){
                o[key]=parseString(s,error);
                atVal=false;
            }else{
                key=parseString(s,error);
                atVal=true;
            }
        } else if (c=='['){
            if(atVal){
                o[key]=parseArray(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        } else if (c=='{'){
            if(atVal){
                o[key]=parseObject(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        } else {
            if(atVal){
                o[key]=parseLiteral(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        }
        s.skipWhiteSpace();
    }
    error=true;
    return QVariantMap();
}
static QVariantList parseArray (QTextStream & s,bool & error){
    s.skipWhiteSpace();
    QVariantList l;
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c==']'){
            return l;
        } else if (c==','){
        } else if (c=='"'){
            l.append(parseString(s,error));
        } else if (c=='['){
            l.append(parseArray(s,error));
        } else if (c=='{'){
            l.append(parseObject(s,error));
        } else {
            l.append(parseLiteral(s,error));
        }
        s.skipWhiteSpace();
    }
    error=true;
    return QVariantList();
}
static QString parseString (QTextStream & s,bool & error){
    QString str;
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if(c=='"'){
            return str;
        }else if(c=='\\'){
            s>>c;
            if(c=='b'){
                str.append('\b');
            }else if(c=='f'){
                str.append('\f');
            }else if(c=='n'){
                str.append('\n');
            }else if(c=='r'){
                str.append('\r');
            }else if(c=='t'){
                str.append('\t');
            }else if(c=='f'){
                str.append('\f');
            }else if(c=='u'){

                //Big thanks arsi for this fix!
//                short u;
//                s>>u;
//                str.append(QChar(u));
                str.append(QChar(s.read(4).toUShort(0, 16)));

            }else{
                str.append(c);
            }
        }else{
            str.append(c);
        }
    }
    error=true;
    return QString();
}
static QVariant parseLiteral (QTextStream & s,bool & error){
    s.seek(s.pos()-1);
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c=='t'){
            s>>c;//r
            s>>c;//u
            s>>c;//e
            return true;
        } else if (c=='f'){
            s>>c;//a
            s>>c;//l
            s>>c;//s
            s>>c;//e
            return false;
        }else if (c=='n'){
            s>>c;//u
            s>>c;//l
            s>>c;//l
            return QVariant();
        }else if (c=='-' || c.isDigit()){
            QString n;
            while(( c.isDigit()  || (c=='.') || (c=='E') || (c=='e') || (c=='-') || (c=='+') )){
                n.append(c);
                if(s.atEnd() ||  error)
                    break;
                s>>c;
            }
            s.seek(s.pos()-1);
            if(n.contains('.')){
                return n.toDouble();
            }else{
                return n.toInt();
            }
        }
    }
    error=true;
    return QVariant();
}
