#include "FluidParticipantImpl.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>

namespace MinimalCoupler
{

    FluidParticipantImplementation::FluidParticipantImplementation(std::string participantName,
    std::string configurationFileName,int solverProcessIndex, int solverProcessSize)
        : ParticipantImplementation(participantName, configurationFileName, solverProcessIndex, solverProcessSize), solidSocket(-1)
    {

    }

    void FluidParticipantImplementation::initialize()
    {
        //1. Preprocess any meshes

        //2. Setup communication between Fluid and solid participants
        solidSocket = getSolidConnectionSocket();
        //3. Transfer vertices from participant sender to receiver

        //4. Initialize the coupling scheme

        //5. Transfer Write Data

        //6. Transfer Read Data
    }


    int FluidParticipantImplementation::getSolidConnectionSocket()
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family  = AF_INET;
        serverAddress.sin_port = htons(5001);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0)
        {
            close(sock);
            throw std::runtime_error("Failed to bind socket");
        }

        if (listen(sock, 1) < 0)
        {
            close(sock);
            throw std::runtime_error("Unable to listen on socket");
        }

        int solidSocket = accept(sock, nullptr, nullptr);
        if (solidSocket < 0)
        {
            close(sock);
            throw std::runtime_error("Unable to accept connection request from the client");
        }

        close(sock);
        return solidSocket;
    }
}