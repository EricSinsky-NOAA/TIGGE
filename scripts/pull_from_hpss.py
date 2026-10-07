#! /bin/env python3

import datetime
import os
import subprocess
from functools import partial
import argparse
import contextlib

# Make sure print statements are flushed immediately, otherwise
#   print statments may be out-of-order with subprocess output
print = partial(print, flush=True)

htar_command = 'htar -xvf {file}'
destination_pattern = '{com}/gefs.%Y%m%d/%H'


def get_pgrb2a_pattern(time: datetime.datetime) -> str:
	if time in [datetime.datetime(2023, 8, 6, 6), datetime.datetime(2023, 8, 10, 18)]:
		# One-off manual archival by Eric
		return time.strftime('/2year/NCEPDEV/emc-ensemble/Eric.Sinsky/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
	elif datetime.datetime(2018, 8, 1, 0) <= time < datetime.datetime(2019, 8, 20, 0):
		return time.strftime('/NCEPPROD/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com2_gens_prod_gefs.%Y%m%d_%H.pgrb2a.tar')
	elif time < datetime.datetime(2020, 2, 26, 0):
		return time.strftime('/NCEPPROD/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/gpfs_dell2_nco_ops_com_gens_prod_gefs.%Y%m%d_%H.pgrb2a.tar')
	elif time < datetime.datetime(2020, 9, 23, 12):
		return time.strftime('/NCEPPROD/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gens_prod_gefs.%Y%m%d_%H.pgrb2a.tar')
	elif time < datetime.datetime(2022, 6, 27, 0):
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_prod_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/2year/NCEPDEV/emc-ensemble/emc.enspara1/Xianwu.Xue/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_prod_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
	elif time < datetime.datetime(2022, 10, 18, 0):
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.2_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/NCEPDEV/emc-ensemble/2year/emc.ens/xianwu.xue/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.2_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
	elif time < datetime.datetime(2024, 4, 1, 0):
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/NCEPDEV/emc-ensemble/2year/emc.ens/xianwu.xue/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
	elif time < datetime.datetime.now():
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/NCEPDEV/emc-ensemble/2year/emc.ens/eric.sinsky/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2ap5.tar')
	else:
		raise Exception(time.strftime("Pattern not defined for this date: %Y%m%d_%H"))


def get_pgrb2b_pattern(time: datetime.datetime) -> str:
	if time in [datetime.datetime(2023, 8, 6, 6)]:
		# One-off manual archival by Eric
		return time.strftime('/2year/NCEPDEV/emc-ensemble/Eric.Sinsky/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
	elif datetime.datetime(2018, 8, 1, 0) <= time < datetime.datetime(2019, 8, 20, 0):
		return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com2_gens_prod_gefs.%Y%m%d_%H.pgrb2b.tar')
	elif time < datetime.datetime(2020, 2, 26, 0):
		return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/gpfs_dell2_nco_ops_com_gens_prod_gefs.%Y%m%d_%H.pgrb2b.tar')
	elif time < datetime.datetime(2020, 9, 23, 12):
		return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gens_prod_gefs.%Y%m%d_%H.pgrb2b.tar')
	elif time < datetime.datetime(2022, 6, 27, 0):
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_prod_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/2year/NCEPDEV/emc-ensemble/emc.enspara1/Xianwu.Xue/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_prod_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
	elif time < datetime.datetime(2022, 10, 18, 0):
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.2_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/NCEPDEV/emc-ensemble/2year/emc.ens/xianwu.xue/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.2_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
	elif time < datetime.datetime(2024, 4, 1, 0):
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/NCEPDEV/emc-ensemble/2year/emc.ens/xianwu.xue/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
	elif time < datetime.datetime.now():
		if time.hour in [0, 12]:
			return time.strftime('/NCEPPROD/2year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
		elif time.hour in [6, 18]:
			return time.strftime('/NCEPDEV/emc-ensemble/2year/emc.ens/eric.sinsky/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2bp5.tar')
	else:
		raise Exception(time.strftime("Pattern not defined for this date: %Y%m%d_%H"))


def get_pgrb2s_pattern(time: datetime.datetime) -> str:
    if time < datetime.datetime.now():
        if time.hour in [0, 6, 12, 18]:
            return time.strftime('/NCEPPROD/5year/hpssprod/runhistory/rh%Y/%Y%m/%Y%m%d/com_gefs_v12.3_gefs.%Y%m%d_%H.atmos_pgrb2sp25.tar')
    else:
        raise Exception(time.strftime("Pattern not defined for this date: %Y%m%d_%H"))


def main():
	parser = argparse.ArgumentParser(description="Pulls 1.0 deg GEFS products from HPSS for TIGGE")
	parser.add_argument('time', type=lambda s: datetime.datetime.strptime(s, "%Y%m%d%H"), help="Cycle date in YYYYMMDDHH format")

	time = parser.parse_args().time

	com = os.environ.get('GEFS_COM')
	if com is None:
		print(f"FATAL ERROR in {__file__}: GEFS_COM is not defined!")
		quit(100)

	destination = time.strftime(destination_pattern.format(com=com))

	with contextlib.suppress(FileExistsError):
		os.makedirs(destination)

	os.chdir(destination)

	subprocess.check_call(htar_command.format(file=get_pgrb2a_pattern(time)), shell=True)
	subprocess.check_call(htar_command.format(file=get_pgrb2b_pattern(time)), shell=True)
	subprocess.check_call(htar_command.format(file=get_pgrb2s_pattern(time)), shell=True)

	print(f"{__file__} completed successfully")


if __name__ == "__main__":
	main()
