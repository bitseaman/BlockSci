import blocksci

chain = blocksci.Blockchain("../zcash-data") # Your folder, where the parsed Zcash blockchain data is found

sapling = 0
sapling_hidden = 0
sapling_revealed = 0
sapling_spends = 0
sapling_outputs = 0

sprout = 0
sprout_hidden = 0
sprout_revealed = 0


difficulties = []

for blk in chain[:450000]:
	difficulties.append(blk.difficulty) # Difficulty of a block
	for tx in blk:
		if tx.is_shielded:
			if tx.is_sproutshielded:
				sprout += 1
				sprout_hidden += tx.sum_vpubold
				sprout_revealed += tx.sum_vpubnew
			if tx.is_saplingshielded:
				sapling += 1
				sapling_spends += tx.sspend_count # Number of shielded spends in the transaction
				sapling_outputs += tx.soutput_count # Number of shielded outputs in the transaction
				if tx.value_balance < 0:
					sapling_hidden += abs(tx.value_balance)
				else:
					sapling_revealed += tx.value_balance


print("ZEC in sprout addresses:")
print((sprout_hidden - sprout_revealed) / 100000000) # Values are stored in Zatoshis
print("ZEC in sapling addresses:")
print((sapling_hidden - sapling_revealed) / 100000000)