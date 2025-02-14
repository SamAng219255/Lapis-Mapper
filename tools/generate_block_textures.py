import sys
import json
from PIL import Image
from os import path
import re

printHash=True

basePath=r"/Users/samanguiano/Library/Application Support/minecraft/versions/24w36a/24w36a/assets/minecraft/textures/block/"
basePathI=r"/Users/samanguiano/Library/Application Support/minecraft/versions/24w36a/24w36a/assets/minecraft/textures/item/"
basePathE=r"/Users/samanguiano/Library/Application Support/minecraft/versions/24w36a/24w36a/assets/minecraft/textures/entity/"

missingBlocks=[]

def hash(strng):
	hash = 5381
	c=0
	for char in strng:
		hash = (((hash << 5)%18446744073709551616 + hash)%18446744073709551616 ^ ord(char))%18446744073709551616
	return hash


def main():
	for blockName in blockNames:
		usedPath=basePath
		blockNameUsed=blockName
		if 'bubble_column'==blockNameUsed:
			blockNameUsed='water'
		if blockNameUsed.endswith('_stairs'):
			blockNameUsed=re.sub('_stairs','',blockNameUsed)
		if blockNameUsed.endswith('_slab'):
			blockNameUsed=re.sub('_slab','',blockNameUsed)
		if blockNameUsed.endswith('_wall'):
			blockNameUsed=re.sub('_wall','',blockNameUsed)
		if blockNameUsed.endswith('_button'):
			blockNameUsed=re.sub('_button','',blockNameUsed)
		if blockNameUsed.endswith('_fence'):
			blockNameUsed=re.sub('_fence','',blockNameUsed)
		if blockNameUsed.endswith('_fence_gate'):
			blockNameUsed=re.sub('_fence_gate','',blockNameUsed)
		if blockNameUsed.endswith('_wall_sign'):
			blockNameUsed=re.sub('_wall_sign','',blockNameUsed)
		if blockNameUsed.endswith('_wall_hanging_sign'):
			blockNameUsed=re.sub('_wall_hanging_sign','',blockNameUsed)
		if blockNameUsed.endswith('_hanging_sign'):
			blockNameUsed=re.sub('_hanging_sign','',blockNameUsed)
		if blockNameUsed.endswith('_sign'):
			blockNameUsed=re.sub('_sign','',blockNameUsed)
		if blockNameUsed.endswith('_pressure_plate'):
			blockNameUsed=re.sub('_pressure_plate','',blockNameUsed)
		if blockNameUsed.endswith('_pane'):
			blockNameUsed=re.sub('_pane','',blockNameUsed)
		if blockNameUsed.endswith('kelp_block'):
			blockNameUsed=re.sub('_block','',blockNameUsed)
		if blockNameUsed.endswith('_wood'):
			blockNameUsed=re.sub('_wood','',blockNameUsed)+"_log"
		if blockNameUsed.endswith('_hyphae'):
			blockNameUsed=re.sub('_hyphae','',blockNameUsed)+"_stem"
		if blockNameUsed.endswith('_bed'):
			blockNameUsed=re.sub('_bed','',blockNameUsed)+"_wool"
		if blockNameUsed.endswith('_carpet') and not blockNameUsed.startswith('moss'):
			blockNameUsed=re.sub('_carpet','',blockNameUsed)+"_wool"
		if blockNameUsed.endswith('_carpet'):
			blockNameUsed=re.sub('_carpet','',blockNameUsed)+"_block"
		if blockNameUsed.endswith('_wall_banner'):
			blockNameUsed=re.sub('_wall_banner','',blockNameUsed)+"_wool"
		if blockNameUsed.endswith('_banner'):
			blockNameUsed=re.sub('_banner','',blockNameUsed)+"_wool"
		if blockNameUsed.endswith('_wall_fan'):
			blockNameUsed=re.sub('_wall_fan','',blockNameUsed)+"_fan"
		if blockNameUsed.endswith('_wire'):
			blockNameUsed=re.sub('_wire','',blockNameUsed)+"_dust_dot"
		if blockNameUsed.endswith('wall_torch'):
			blockNameUsed=re.sub('wall_torch','',blockNameUsed)+"torch"
		if blockNameUsed.endswith('campfire'):
			blockNameUsed=re.sub('campfire','',blockNameUsed)+"fire"
		if blockNameUsed.startswith('potted_'):
			blockNameUsed=blockNameUsed[len('potted_'):]
		if blockNameUsed.startswith('infested_'):
			blockNameUsed=blockNameUsed[len('infested_'):]
		if blockNameUsed.startswith('waxed_'):
			blockNameUsed=blockNameUsed[len('waxed_'):]
		if blockNameUsed.startswith('chipped_'):
			blockNameUsed=blockNameUsed[len('chipped_'):]
		if blockNameUsed.startswith('damaged_'):
			blockNameUsed=blockNameUsed[len('damaged_'):]
		if blockNameUsed.startswith('sticky_'):
			blockNameUsed=blockNameUsed[len('sticky_'):]
		if blockNameUsed.startswith('moving_'):
			blockNameUsed=blockNameUsed[len('moving_'):]
		if blockNameUsed.startswith('trapped_'):
			blockNameUsed=blockNameUsed[len('trapped_'):]
		if blockNameUsed.startswith('ender_'):
			blockNameUsed=blockNameUsed[len('ender_'):]
		if blockNameUsed.startswith('frosted_'):
			blockNameUsed=blockNameUsed[len('frosted_'):]
		if blockNameUsed.startswith('petrified_'):
			blockNameUsed=blockNameUsed[len('petrified_'):]
		if blockNameUsed.startswith('pointed_'):
			blockNameUsed=blockNameUsed[len('pointed_'):]
		if blockNameUsed.startswith('snow_'):
			blockNameUsed='snow'
		if blockNameUsed.startswith('magma_'):
			blockNameUsed='magma'
		if blockNameUsed.startswith('suspicious_'):
			blockNameUsed+='_0'
		if blockNameUsed.endswith('azalea_bush'):
			blockNameUsed=re.sub('_bush','',blockNameUsed)
		if blockNameUsed.endswith('brick'):
			blockNameUsed+='s'
		if blockNameUsed.endswith('tile'):
			blockNameUsed+='s'
		if blockNameUsed.endswith('fire'):
			blockNameUsed+='_0'
		if blockNameUsed.endswith('dripstone'):
			blockNameUsed+='_block'
		if blockNameUsed.endswith('lava') or blockNameUsed.endswith('water'):
			blockNameUsed+='_still'
		if blockNameUsed.endswith('oak') or blockNameUsed.endswith('spruce') or blockNameUsed.endswith('birch') or blockNameUsed.endswith('jungle') or blockNameUsed.endswith('acacia') or blockNameUsed.endswith('dark_oak') or blockNameUsed.endswith('cherry') or blockNameUsed.endswith('mangrove') or blockNameUsed.endswith('crimson') or blockNameUsed.endswith('warped'):
			blockNameUsed+='_planks'
		if blockNameUsed.endswith('quartz') or blockNameUsed.endswith('purpur') or blockNameUsed.endswith('moss'):
			blockNameUsed+='_block'
		if blockNameUsed.startswith('smooth_') and not blockNameUsed.startswith('smooth_stone_'):
			blockNameUsed=blockNameUsed[len('smooth_'):]
		if 'decorated_pot' in blockNameUsed:
			blockNameUsed='terracotta'
		if 'pitcher_plant' in blockNameUsed:
			blockNameUsed='pitcher_crop_top_stage_4'
		if 'cake' in blockNameUsed:
			blockNameUsed='cake_top'
		if 'cauldron' in blockNameUsed:
			blockNameUsed='cauldron'
		if 'bamboo' in blockNameUsed:
			blockNameUsed='bamboo_stalk'
		if 'head' in blockNameUsed or 'skull' in blockNameUsed:
			blockNameUsed='soul_sand'
		if 'carrots'==blockNameUsed or 'potatoes'==blockNameUsed or 'wheat'==blockNameUsed or 'nether_wart'==blockNameUsed or 'beetroots'==blockNameUsed:
			usedPath=basePathI
			if blockNameUsed.endswith('es'):
				blockNameUsed=re.sub('es','',blockNameUsed)
			if blockNameUsed.endswith('s'):
				blockNameUsed=re.sub('s','',blockNameUsed)
		if 'chest' in blockNameUsed:
			usedPath=basePathE+'chest/'
			if blockNameUsed=='chest':
				blockNameUsed='normal'
			if blockNameUsed=='ender_chest':
				blockNameUsed='ender'
			if blockNameUsed=='trapped_chest':
				blockNameUsed='trapped'
		if 'cocoa'==blockNameUsed:
			blockNameUsed+='_stage2'
		if 'grindstone'==blockNameUsed:
			blockNameUsed+='_side'
		if 'sweet_berry_bush'==blockNameUsed:
			blockNameUsed+='_stage3'
		if 'sniffer_egg'==blockNameUsed:
			blockNameUsed+='_not_cracked'
		if 'light_weighted' in blockNameUsed:
			blockNameUsed='iron_block'
		if 'heavy_weighted' in blockNameUsed:
			blockNameUsed='gold_block'
		if 'torchflower_crop'==blockNameUsed:
			blockNameUsed='torchflower'
		if 'trial_spawner'==blockNameUsed:
			blockNameUsed='trial_spawner_top_inactive'
		filepath=usedPath+blockNameUsed+".png"
		#print(filepath)
		if not path.exists(filepath):
			filepath=usedPath+blockNameUsed+'_top'+".png"
		if not path.exists(filepath):
			filepath=usedPath+blockNameUsed+'_front'+".png"
		if path.exists(filepath):
			with Image.open(filepath) as img:
				Rtotal=0
				Gtotal=0
				Btotal=0
				total=0
				for count, (R, G, B, A) in img.convert("RGBA").getcolors(2048):
					Rtotal+=R*count*A
					Gtotal+=G*count*A
					Btotal+=B*count*A
					total+=count*A
				Rtotal/=total
				Gtotal/=total
				Btotal/=total
				if printHash: print('{:016x}{:02x}{:02x}{:02x}'.format(hash(blockName),int(Rtotal),int(Gtotal),int(Btotal)))
				else: print(blockName+': "#{:02x}{:02x}{:02x}"'.format(int(Rtotal),int(Gtotal),int(Btotal)))
		else:
			missingBlocks.append(blockName)
	for block in missingBlocks:
		if printHash: print('{:016x}{:02x}{:02x}{:02x}'.format(hash(block),int(0),int(0),int(0)))
		else: print(block+': "#{:02x}{:02x}{:02x}"'.format(int(0),int(0),int(0)))


with open(sys.argv[1]) as f:
	blockNames = json.load(f)
	main()