//
// Created by pwootage on 8/19/16.
//

#include "PrimeMemoryDumping.h"

#define GET_PTR(value) (P2C(value))
#define MEM_HIGH (0x2400000)
#define INVALID_PTR(value) (value) > MEM_HIGH

#define PRIME_1_GAMEID (0x474D3845)
#define PRIME_1_MAKERID (0x3031)

void primeMemoryDump(PrimeMemoryDump *dest) {
  u32 gameID = read32FromGCMemory(0x00);
  u16 makerID = read16FromGCMemory(0x04);

  dest->gameid = gameID;
  dest->makerid = makerID;
  dest->type = PACKET_TYPE_INVALID;

  if (gameID == PRIME_1_GAMEID && makerID == PRIME_1_MAKERID) {
    u32 playerStatusPtr = read32FromGCMemory(0x4578CC);
    u32 currentWorldPtr = read32FromGCMemory(0x45A9F8);

    u32 cPlayerPtr = 0x46B97C;
    u32 cPlayerTransformPtr = cPlayerPtr + 0x34;
    u32 cPlayerVelocityPtr = cPlayerPtr + 0x138;
    u32 playerCollisionPrimitvePtr = cPlayerPtr + 0x1c0;
    u32 playerAABBPtr = playerCollisionPrimitvePtr + 0x10;
    u32 playerTranslationPtr = cPlayerPtr + 0x1f4;

    u32 cPlayerMorphStatePtr = cPlayerPtr + 0x2f8;
    u32 cMorphBallPtr = read32FromGCMemory(cPlayerPtr + 0x768);
    u32 morphCollisionPrimitvePtr = cMorphBallPtr + 0x38;
    u32 morphVecPtr = morphCollisionPrimitvePtr + 0x10;
    u32 morphRadiusPtr = morphCollisionPrimitvePtr + 0x1c;

    dest->type = PACKET_TYPE_GAME_DATA;
    dest->speed[0] = read32FromGCMemory(cPlayerVelocityPtr + 0x0);
    dest->speed[1] = read32FromGCMemory(cPlayerVelocityPtr + 0x4);
    dest->speed[2] = read32FromGCMemory(cPlayerVelocityPtr + 0x8);
    dest->pos[0] = readFloatFromGCMemory(cPlayerTransformPtr + 0xC);
    dest->pos[1] = readFloatFromGCMemory(cPlayerTransformPtr + 0x1C);
    dest->pos[2] = readFloatFromGCMemory(cPlayerTransformPtr + 0x2C);
//    float playerXOffset = readFloatFromGCMemory(playerTranslationPtr + 0x0);
//    float playerYOffset = readFloatFromGCMemory(playerTranslationPtr + 0x4);
//    float playerZOffset = readFloatFromGCMemory(playerTranslationPtr + 0x8);
    float playerXOffset = dest->pos[0];
    float playerYOffset = dest->pos[1];
    float playerZOffset = dest->pos[2];
    dest->playerAABB[0] = readFloatFromGCMemory(playerAABBPtr + 0x0) + playerXOffset;
    dest->playerAABB[1] = readFloatFromGCMemory(playerAABBPtr + 0x4) + playerYOffset;
    dest->playerAABB[2] = readFloatFromGCMemory(playerAABBPtr + 0x8) + playerZOffset;
    dest->playerAABB[3] = readFloatFromGCMemory(playerAABBPtr + 0xC) + playerXOffset;
    dest->playerAABB[4] = readFloatFromGCMemory(playerAABBPtr + 0x10) + playerYOffset;
    dest->playerAABB[5] = readFloatFromGCMemory(playerAABBPtr + 0x14) + playerZOffset;
    dest->morphStatus = read32FromGCMemory(cPlayerMorphStatePtr);
    dest->morphedPos[0] = readFloatFromGCMemory(morphVecPtr + 0x0) + playerXOffset;
    dest->morphedPos[1] = readFloatFromGCMemory(morphVecPtr + 0x4) + playerYOffset;
    dest->morphedPos[2] = readFloatFromGCMemory(morphVecPtr + 0x8) + playerZOffset;
    dest->morphedRadius = readFloatFromGCMemory(morphRadiusPtr);
    dest->worldID = read32FromGCMemory(currentWorldPtr + 0x08);
    dest->worldStatus = read32FromGCMemory(currentWorldPtr + 0x04);
    dest->room = read32FromGCMemory(0x45AA74);
    dest->health = read32FromGCMemory(playerStatusPtr + 0x2AC);
    u32 i;
    for (i = 0; i < INVENTORY_SIZE; i++) {
      dest->inventory[i] = read32FromGCMemory(playerStatusPtr + 0x2C8 + i * 4);
    }
    dest->timer = read64FromGCMemory(playerStatusPtr + 0xA0);
  }
};

float readFloatFromGCMemory(u32 addr) {
  u32 actualPtr = P2C(addr);
  if (INVALID_PTR(actualPtr)) {
    return 0;
  }
  u32 res = read32(GET_PTR(actualPtr));
  return *((float *) (&res));
}

u64 read64FromGCMemory(u32 addr) {
  u32 actualPtr = P2C(addr);
  if (INVALID_PTR(actualPtr)) {
    return 0;
  }
  u64 res = ((u64) read32(GET_PTR(actualPtr)) << 32) | (u64) read32(GET_PTR(actualPtr + 4));
  return res;
}

u32 read32FromGCMemory(u32 addr) {
  u32 actualPtr = P2C(addr);
  if (INVALID_PTR(actualPtr)) {
    return 0;
  }
  u32 res = read32(GET_PTR(actualPtr));
  return res;
}

u16 read16FromGCMemory(u32 addr) {
  u32 actualPtr = P2C(addr);
  if (INVALID_PTR(actualPtr)) {
    return 0;
  }
  u16 res = read16(GET_PTR(actualPtr));
  return res;
}

u8 read8FromGCMemory(u32 addr) {
  u32 actualPtr = P2C(addr);
  if (INVALID_PTR(actualPtr)) {
    return 0;
  }
  u8 res = read8(GET_PTR(actualPtr));
  return res;
}