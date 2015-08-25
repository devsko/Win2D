// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the MIT License. See LICENSE.txt in the project root for license information.

#pragma once

class RegisteredEvent
{
    std::function<void()> m_unregisterFunction;

public:
    template<typename DELEGATE, typename SOURCE, typename HANDLER>
    RegisteredEvent(
        SOURCE* eventSource,
        HRESULT (STDMETHODCALLTYPE SOURCE::* addMethod)(DELEGATE*, EventRegistrationToken*),
        HRESULT (STDMETHODCALLTYPE SOURCE::* removeMethod)(EventRegistrationToken),
        HANDLER handler)
    {
        EventRegistrationToken token;
        ThrowIfFailed((eventSource->*addMethod)(handler, &token));

        Microsoft::WRL::ComPtr<SOURCE> source(eventSource);
        m_unregisterFunction = 
            [source, removeMethod, token]()
            {
                ThrowIfFailed((source.Get()->*removeMethod)(token));
            };
    }

    RegisteredEvent(std::function<void()>&& unregisterFunction)
        : m_unregisterFunction(std::move(unregisterFunction))
    {
    }

    RegisteredEvent()
    {
    }

    RegisteredEvent(RegisteredEvent&& other)
    {
        std::swap(m_unregisterFunction, other.m_unregisterFunction);
    }

    RegisteredEvent& operator=(RegisteredEvent&& other)
    {
        Release();
        m_unregisterFunction = other.m_unregisterFunction;
        other.m_unregisterFunction = nullptr;
        return *this;
    }

    RegisteredEvent(const RegisteredEvent&) = delete;
    RegisteredEvent& operator=(const RegisteredEvent&) = delete;

    ~RegisteredEvent()
    {
        Release();
    }

    void Detach()
    {
        m_unregisterFunction = nullptr;
    }

    void Release()
    {
        if (m_unregisterFunction)
        {
            m_unregisterFunction();
            m_unregisterFunction = nullptr;
        }
    }

    explicit operator bool()
    {
        return m_unregisterFunction != nullptr;
    }
};
