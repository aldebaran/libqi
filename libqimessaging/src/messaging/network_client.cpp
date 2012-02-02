/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/network_client.hpp>

NetworkClient::NetworkClient()
{
}

NetworkClient::~NetworkClient()
{
}

void NetworkClient::connect(const std::string masterAddress)
{
}

bool NetworkClient::disconnect()
{
}


bool NetworkClient::waitForConnected(int msecs = 30000)
{
}

bool NetworkClient::waitForDisconnected(int msecs = 30000)
{
}


void NetworkClient::registerMachine(const qi::MachineInfo& m)
{
}

void NetworkClient::unregisterMachine(const qi::MachineInfo& m)
{
}


bool NetworkClient::isInitialized() const
{
}


