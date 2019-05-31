import blocksci


chain = blocksci.Blockchain("../zcash-data") #Your folder, where the parsed Zcash blockchain data is found

sapling = 0
sapling_hidden = 0
sapling_revealed = 0
sapling_spends = 0
sapling_outputs = 0

sprout = 0
sprout_hidden = 0
sprout_revealed = 0


difficulties = []

for blk in chain:
	difficulties.append(blk.difficulty)
	for tx in blk:
		if tx.is_shielded:
			if tx.is_sproutshielded:
				sprout += 1
				sprout_hidden += tx.sum_vpubold
				sprout_revealed += tx.sum_vpubnew
			if tx.is_saplingshielded:
				sapling += 1
				sapling_spends += tx.sspend_count #number of sapling spend notes
				sapling_outputs += tx.soutput_count #number of sapling output notes
				if tx.value_balance < 0:
					sapling_hidden += abs(tx.value_balance)
				else:
					sapling_revealed += tx.value_balance