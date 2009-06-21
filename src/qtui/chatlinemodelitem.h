/***************************************************************************
 *   Copyright (C) 2005-09 by the Quassel Project                          *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CHATLINEMODELITEM_H_
#define CHATLINEMODELITEM_H_

#include "messagemodel.h"

#include "uistyle.h"

class ChatLineModelItem : public MessageModelItem {
public:
  ChatLineModelItem(const Message &);

  virtual QVariant data(int column, int role) const;

  virtual inline const Message &message() const { return _styledMsg; }
  virtual inline const QDateTime &timestamp() const { return _styledMsg.timestamp(); }
  virtual inline const MsgId &msgId() const { return _styledMsg.msgId(); }
  virtual inline const BufferId &bufferId() const { return _styledMsg.bufferId(); }
  virtual inline void setBufferId(BufferId bufferId) { _styledMsg.setBufferId(bufferId); }
  virtual inline Message::Type msgType() const { return _styledMsg.type(); }
  virtual inline Message::Flags msgFlags() const { return _styledMsg.flags(); }

  /// Used to store information about words to be used for wrapping
  struct Word {
    quint16 start;
    qreal endX;
    qreal width;
    qreal trailing;
  };
  typedef QVector<Word> WrapList;

private:
  QVariant timestampData(int role) const;
  QVariant senderData(int role) const;
  QVariant contentsData(int role) const;

  QVariant backgroundBrush(UiStyle::FormatType subelement) const;
  quint32 messageLabel() const;

  void computeWrapList() const;

  mutable WrapList _wrapList;
  UiStyle::StyledMessage _styledMsg;

  static unsigned char *TextBoundaryFinderBuffer;
  static int TextBoundaryFinderBufferSize;
};

#endif
