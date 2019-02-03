#include "ObfuscationMgr.h"

//In case you don't want to Include the whole directx library
#if !defined(D3DX11_SDK_VERSION)
typedef struct D3D11_MAPPED_SUBRESOURCE
{
	void *pData;
	UINT RowPitch;
	UINT DepthPitch;
} 	D3D11_MAPPED_SUBRESOURCE;
typedef 
enum D3D11_MAP
{
	D3D11_MAP_READ	= 1,
	D3D11_MAP_WRITE	= 2,
	D3D11_MAP_READ_WRITE	= 3,
	D3D11_MAP_WRITE_DISCARD	= 4,
	D3D11_MAP_WRITE_NO_OVERWRITE	= 5
} D3D11_MAP;
struct ID3D11DeviceContext
{
	virtual void Function0(); //
	virtual void Function1(); //
	virtual void Function2(); //
	virtual void Function3(); //
	virtual void Function4(); //
	virtual void Function5(); //
	virtual void Function6(); //
	virtual void Function7(); //
	virtual void Function8(); //
	virtual void Function9(); //
	virtual void Function10(); //
	virtual void Function11(); //
	virtual void Function12(); //
	virtual void Function13(); //
	virtual HRESULT STDMETHODCALLTYPE Map( void *pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource);
	virtual void STDMETHODCALLTYPE Unmap(  void *pResource, UINT Subresource);
};
struct ID3D11Buffer
{
};
#endif

struct ObfuscationMgr
{
	char _0x0000[224];
	__int64 m_E0; //0x00E0 
	char _0x00E8[4];
	bool IsMultiPlayer; //0x00EC 
	char _0x00ED[3];
	__int64 m_dwUpdateTimer; //0x00F0 
	char _0x00F8[8];
	__int64 m_DecryptionFunction; //0x0100 
	__int64 m_EncryptedBuffer; //0x0108 
	__int64 m_EncryptedDeviceContext; //0x0110 
	__int64 m_EncryptedDevice; //0x0118 
	__int64 m_D3d11; //0x0120 
};

__int64 __fastcall PointerXorMultiplayer(__int64 ValueToXor /*RCX*/, __int64 EncryptedBuffer /*RDX*/, __int64 EncryptedDeviceContext /*R8*/ )
{
	__int64 XorD3D11 = 0xAB541E6F771275BCui64;

	ID3D11DeviceContext* pDeviceContext = (ID3D11DeviceContext*)( EncryptedDeviceContext ^ XorD3D11 );
	ID3D11Buffer* pBuffer = (ID3D11Buffer*)( EncryptedBuffer ^ XorD3D11 );

	D3D11_MAPPED_SUBRESOURCE MappedSubResource = {0};

	HRESULT hResult = pDeviceContext->Map( pBuffer, NULL, D3D11_MAP_READ, NULL, &MappedSubResource );
	if ( !SUCCEEDED(hResult) || !MappedSubResource.pData )
		return ValueToXor;

	__int64 XorKey = *(__int64 *)MappedSubResource.pData;

	pDeviceContext->Unmap( pBuffer, NULL );

	return ValueToXor ^ XorKey;
}

__int64 __fastcall PointerXorSinglePlayer(__int64 RCX )
{
	return RCX ^ 0x598447EFD7A36912i64;
}

//So its 2019 if you still use a shitty external in c# you can do the following:
//	Before you join any server make sure your external is running.
//	You need to keep the UpdateTimer inside the ObfuscationMgr below 24000
//	then the ObfuscationMgr will NEVER activate the multiplayer encryption!

void BypassObfuscationMgr()
{
	_QWORD pObfuscationMgr = *(_QWORD*)OFFSET_ObfuscationMgr;
	if (!ValidPointer(pObfuscationMgr)) return;

	if ( *(__int64*)( pObfuscationMgr + 0x108 ) != NULL )
		return;

	//pObfuscationMgr->m_dwUpdateTimer = 0
	*(__int64*)( pObfuscationMgr + 0xF0 ) = 0ui64; 
}

_QWORD __fastcall PointerXor(_QWORD RCX, _QWORD RDX)
{
//original function code from sub_1415780F0
//RAX = ( *(_QWORD *)(RCX + 0xD8) ^ *(_QWORD *)(RCX + 0xF8) )
//RCX = RDX
//RDX = *(_QWORD *)(RCX + 0x100)
//R8  = *(_QWORD *)(RCX + 0x108)
//jmp RAX

	_QWORD pObfuscationMgr = RCX - 8;

	_QWORD EncryptedBuffer = *(_QWORD *)(pObfuscationMgr + 0x108);
	_QWORD EncryptedDeviceContext = *(_QWORD *)(pObfuscationMgr + 0x110);

	DWORD64 DecryptFunction = ( *(_QWORD *)(pObfuscationMgr + 0xE0) ^ *(_QWORD *)(pObfuscationMgr + 0x100) );
	
	if ( *(bool*)(pObfuscationMgr + 0xEC) != true || EncryptedBuffer == NULL || EncryptedDeviceContext == NULL )
		return (_QWORD)PointerXorSinglePlayer( RDX );
	
	return (_QWORD)PointerXorMultiplayer( RDX, EncryptedBuffer, EncryptedDeviceContext );
}
 
fb::ClientPlayer* EncryptedPlayerMgr__GetPlayer( QWORD EncryptedPlayerMgr, int id )
{
	_QWORD XorValue1 = *(_QWORD *)(EncryptedPlayerMgr + 0x20) ^ *(_QWORD *)(EncryptedPlayerMgr + 8);
	_QWORD XorValue2 = PointerXor(    *(_QWORD *)(EncryptedPlayerMgr + 0x28),
									  *(_QWORD *)(EncryptedPlayerMgr + 0x10) );
	if (!ValidPointer(XorValue2)) return nullptr;
	_QWORD Player = XorValue1 ^ *(_QWORD *)( XorValue2 + 8 * id);
	return (fb::ClientPlayer*)Player;
}
fb::ClientPlayer* GetPlayerById( int id )
{
	fb::ClientGameContext* pClientGameContext = fb::ClientGameContext::GetInstance();
	if (!ValidPointer(pClientGameContext)) return nullptr;
	fb::ClientPlayerManager* pPlayerManager = pClientGameContext->m_clientPlayerManager;
	if (!ValidPointer(pPlayerManager)) return nullptr;
 
	_QWORD pObfuscationMgr = *(_QWORD*)OFFSET_ObfuscationMgr;
	if (!ValidPointer(pObfuscationMgr)) return nullptr;
 
	_QWORD PlayerListXorValue = *(_QWORD*)( (_QWORD)pPlayerManager + 0xF8 );
	_QWORD PlayerListKey = PlayerListXorValue ^ *(_QWORD *)(pObfuscationMgr + 0xE0 );
 
	fb::hashtable<_QWORD>* table = (fb::hashtable<_QWORD>*)(pObfuscationMgr + 8 + 8);
	fb::hashtable_iterator<_QWORD> iterator = {0};
 
	hashtable_find(table, &iterator, PlayerListKey);
	if ( iterator.mpNode == table->mpBucketArray[table->mnBucketCount] )
		return nullptr;
	if (!ValidPointer(iterator.mpNode)) return nullptr;
	_QWORD EncryptedPlayerMgr = (_QWORD)iterator.mpNode->mValue.second;
	if (!ValidPointer(EncryptedPlayerMgr)) return nullptr;
 
	_DWORD MaxPlayerCount = *(_DWORD *)(EncryptedPlayerMgr + 0x18);
	if( MaxPlayerCount != 70u || MaxPlayerCount <= (unsigned int)(id) ) return nullptr;

	return EncryptedPlayerMgr__GetPlayer( EncryptedPlayerMgr, id );
}
 
fb::ClientPlayer* GetLocalPlayer( void )
{
	fb::ClientGameContext* pClientGameContext = fb::ClientGameContext::GetInstance();
	if (!ValidPointer(pClientGameContext)) return nullptr;
	fb::ClientPlayerManager* pPlayerManager = pClientGameContext->m_clientPlayerManager;
	if (!ValidPointer(pPlayerManager)) return nullptr;
 
	_QWORD pObfuscationMgr = *(_QWORD*)OFFSET_ObfuscationMgr;
	if (!ValidPointer(pObfuscationMgr)) return nullptr;
 
	_QWORD LocalPlayerListXorValue = *(_QWORD*)( (_QWORD)pPlayerManager + 0xF0 );
	_QWORD LocalPlayerListKey = LocalPlayerListXorValue ^ *(_QWORD *)(pObfuscationMgr + 0xE0 );
 
	fb::hashtable<_QWORD>* table = (fb::hashtable<_QWORD>*)(pObfuscationMgr + 8 + 8);
	fb::hashtable_iterator<_QWORD> iterator = {0};
 
	hashtable_find(table, &iterator, LocalPlayerListKey);
	if ( iterator.mpNode == table->mpBucketArray[table->mnBucketCount] )
		return nullptr;
	if (!ValidPointer(iterator.mpNode)) return nullptr;

	_QWORD EncryptedPlayerMgr = (_QWORD)iterator.mpNode->mValue.second;
	if (!ValidPointer(EncryptedPlayerMgr)) return nullptr;
 
	_DWORD MaxPlayerCount = *(_DWORD *)(EncryptedPlayerMgr + 0x18);
	if( MaxPlayerCount != 1u ) return nullptr;
	
	return EncryptedPlayerMgr__GetPlayer( EncryptedPlayerMgr, 0 );
}

void* DecryptPointer( DWORD64 EncryptedPtr, DWORD64 PointerKey )
{
	_QWORD pObfuscationMgr = *(_QWORD*)OFFSET_ObfuscationMgr;
	if (!ValidPointer(pObfuscationMgr))
		return nullptr;
 
	if ( !(EncryptedPtr & 0x8000000000000000) )
		return nullptr; //invalid ptr
 
	_QWORD hashtableKey = *(_QWORD *)(pObfuscationMgr + 0xE0 ) ^ PointerKey;
 
	fb::hashtable<_QWORD>* table = (fb::hashtable<_QWORD>*)( pObfuscationMgr + 0x78 );
	fb::hashtable_iterator<_QWORD> iterator = {};
 
	hashtable_find( table, &iterator, hashtableKey );
	if ( iterator.mpNode == table->mpBucketArray[table->mnBucketCount] ) 
		return nullptr;
	if (!ValidPointer(iterator.mpNode))
		return nullptr;
	_QWORD EncryptionKey = NULL;


	EncryptionKey = PointerXor( (_QWORD)pObfuscationMgr + 8, (_QWORD)(iterator.mpNode->mValue.second) );

	EncryptionKey ^= (5 * EncryptionKey);
 
	_QWORD DecryptedPtr = NULL;
	BYTE* pDecryptedPtrBytes = (BYTE*)&DecryptedPtr;
	BYTE* pEncryptedPtrBytes = (BYTE*)&EncryptedPtr;
	BYTE* pKeyBytes = (BYTE*)&EncryptionKey;
 
	for (char i = 0; i < 7; i++)
	{
		pDecryptedPtrBytes[i] = ( pKeyBytes[i] * 0x3B ) ^ ( pEncryptedPtrBytes[i] + pKeyBytes[i] );
		EncryptionKey += 8;
	}
	pDecryptedPtrBytes[7] = pEncryptedPtrBytes[7];
 
	DecryptedPtr &= ~( 0x8000000000000000 ); //to exclude the check bit
 
	return (void*)DecryptedPtr;
}
