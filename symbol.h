// -----------------------------------------------------------------------
//   Copyright (C) 2009 by Jukka Sirkka
//   jukka.sirkka@iki.fi
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------

#ifndef DEMO_SYMBOL_H
#define DEMO_SYMBOL_H

#include <QString>
#include <QMap>
#include <QList>

namespace Demo {


class Symbol {

    public:

        using TypeList = QList<int>;

        static const int Integer;
        static const int Real;
        static const int Vector;
        static const int Matrix;
        static const int Text;

        const QString& name() const {return mName;}

        virtual int type() const = 0;
        virtual Symbol* clone() const = 0;

        virtual ~Symbol() = default;

    protected:

        Symbol(QString name): mName(std::move(name)) {}

    protected:

        QString mName;
};

typedef QMap<QString, Symbol*> SymbolMap;
typedef QMapIterator<QString, Symbol*> SymbolIterator;


} // namespace Demo

#define CLONEMETHOD(T) T* clone() const override {return new T(*this);}


#endif // DEMO_SYMBOL_H
