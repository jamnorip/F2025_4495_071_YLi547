import logging, sys
from datetime import datetime


from .PlayerPerformanceEvaluation.dailyPlayerPerformanceModel import (
    train_player_win_model,           
    evaluate_and_store_player_vehicle 
)
from .VehiclePerformanceEvaluation.dailyVehiclePerformanceModel import (
    train_vehicle_efficiency          
)

def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
        handlers=[logging.StreamHandler(sys.stdout)]
    )
    logging.info("== Daily job started ==")

    # 1) 
    train_player_win_model(window_months=6, alpha_l2=1.0)

    # 2) 
    evaluate_and_store_player_vehicle(window_months=6, alpha=10)

    # 3)
    train_vehicle_efficiency(window_months=6, alpha_skill=20, min_matches_vehicle=50)

    logging.info("== Daily job finished ==")

if __name__ == "__main__":
    main()